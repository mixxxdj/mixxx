#include "analyzer/analyzerthread.h"

#ifdef __VAMP__
#include "analyzer/analyzerbeats.h"
#include "analyzer/analyzerkey.h"
#endif
#include "analyzer/analyzergain.h"
#include "analyzer/analyzerebur128.h"
#include "analyzer/analyzerwaveform.h"

#include "library/dao/analysisdao.h"

#include "engine/engine.h"

#include "sources/soundsourceproxy.h"
#include "sources/audiosourcestereoproxy.h"

#include "util/db/dbconnectionpooler.h"
#include "util/db/dbconnectionpooled.h"
#include "util/logger.h"
#include "util/timer.h"


namespace {

mixxx::Logger kLogger("AnalyzerThread");

// Analysis is done in blocks.
// We need to use a smaller block size, because on Linux the AnalyzerThread
// can starve the CPU of its resources, resulting in xruns. A block size
// of 4096 frames per block seems to do fine.
constexpr mixxx::AudioSignal::ChannelCount kAnalysisChannels = mixxx::kEngineChannelCount;
constexpr SINT kAnalysisFramesPerBlock = 4096;
const SINT kAnalysisSamplesPerBlock =
        kAnalysisFramesPerBlock * kAnalysisChannels;

// Maximum frequency of progress updates while busy
constexpr std::chrono::milliseconds kBusyProgressInhibitDuration(100);

} // anonymous namespace

AnalyzerThread::AnalyzerThread(
        int id,
        mixxx::DbConnectionPoolPtr pDbConnectionPool,
        UserSettingsPointer pConfig,
        AnalyzerMode mode)
        : WorkerThread(QString("AnalyzerThread %1").arg(id)),
          m_id(id),
          m_pDbConnectionPool(std::move(pDbConnectionPool)),
          m_pConfig(std::move(pConfig)),
          m_mode(mode),
          m_sampleBuffer(kAnalysisSamplesPerBlock),
          m_emittedState(AnalyzerThreadState::Void),
          // The first signal should always be emitted
          m_lastBusyProgressEmittedAt(Clock::now() - kBusyProgressInhibitDuration) {
    m_nextTrack.setValue(TrackPointer());
    m_analyzerProgress.setValue(kAnalyzerProgressUnknown);
    // This type is registered multiple times although once would be sufficient
    qRegisterMetaType<AnalyzerThreadState>();
}

void AnalyzerThread::exec() {
    // The thread-local database connection for waveform analysis
    // must not be closed before returning from this function.
    // Therefore the DbConnectionPooler is defined here independent
    // of whether a database connection will be opened or not.
    mixxx::DbConnectionPooler dbConnectionPooler;

    std::unique_ptr<AnalysisDao> pAnalysisDao;
    if (m_mode != AnalyzerMode::WithBeatsWithoutWaveform) {
        pAnalysisDao = std::make_unique<AnalysisDao>(m_pConfig);
        m_analyzers.push_back(std::make_unique<AnalyzerWaveform>(pAnalysisDao.get()));
    }
    m_analyzers.push_back(std::make_unique<AnalyzerGain>(m_pConfig));
    m_analyzers.push_back(std::make_unique<AnalyzerEbur128>(m_pConfig));
#ifdef __VAMP__
    const bool enforceBpmDetection = m_mode != AnalyzerMode::Default;
    m_analyzers.push_back(std::make_unique<AnalyzerBeats>(m_pConfig, enforceBpmDetection));
    m_analyzers.push_back(std::make_unique<AnalyzerKey>(m_pConfig));
#endif
    DEBUG_ASSERT(!m_analyzers.empty());
    kLogger.debug() << "Activated" << m_analyzers.size() << "analyzers";

    // pAnalysisDao remains null if no analyzer needs database access.
    // Currently only the waveform analyzer makes use of it.
    if (pAnalysisDao) {
        dbConnectionPooler = mixxx::DbConnectionPooler(m_pDbConnectionPool); // move assignment
        if (!dbConnectionPooler.isPooling()) {
            kLogger.warning()
                    << "Failed to obtain database connection for analyzer queue thread";
            return;
        }
        // Obtain and use the newly created database connection within this thread
        QSqlDatabase dbConnection = mixxx::DbConnectionPooled(m_pDbConnectionPool);
        DEBUG_ASSERT(dbConnection.isOpen());
        pAnalysisDao->initialize(dbConnection);
    }

    mixxx::AudioSource::OpenParams openParams;
    openParams.setChannelCount(kAnalysisChannels);

    while (whileIdleAndNotStopped()) {
        DEBUG_ASSERT(m_currentTrack);
        kLogger.debug() << "Analyzing" << m_currentTrack->getLocation();

        // Get the audio
        const auto audioSource =
                SoundSourceProxy(m_currentTrack).openAudioSource(openParams);
        if (!audioSource) {
            kLogger.warning()
                    << "Failed to open file for analyzing:"
                    << m_currentTrack->getLocation();
            emitDoneProgress(kAnalyzerProgressUnknown);
            m_currentTrack.reset();
            continue;
        }

        bool processTrack = false;
        for (auto const& analyzer: m_analyzers) {
            // Make sure not to short-circuit initialize(...)
            if (analyzer->initialize(
                    m_currentTrack,
                    audioSource->sampleRate(),
                    audioSource->frameLength() * kAnalysisChannels)) {
                processTrack = true;
            }
        }

        if (processTrack) {
            const auto analysisResult = analyzeAudioSource(audioSource);
            DEBUG_ASSERT(analysisResult != AnalysisResult::Pending);
            if ((analysisResult == AnalysisResult::Complete) ||
                    (analysisResult == AnalysisResult::Partial)) {
                // The analysis has been finished, and is either complete without
                // any errors or partial if it has been aborted due to a corrupt
                // audio file. In both cases don't reanalyze tracks during this
                // session. A partial analysis would otherwise be repeated again
                // and again, because it is very unlikely that the error vanishes
                // suddenly.
                emitBusyProgress(kAnalyzerProgressFinalizing);
                // This takes around 3 sec on a Atom Netbook
                for (auto const& analyzer: m_analyzers) {
                    analyzer->finalize(m_currentTrack);
                }
                emitDoneProgress(kAnalyzerProgressDone);
            } else {
                for (auto const& analyzer: m_analyzers) {
                    analyzer->cleanup(m_currentTrack);
                }
                emitDoneProgress(kAnalyzerProgressUnknown);
            }
        } else {
            kLogger.debug() << "Skipping track analysis because no analyzer initialized.";
            emitDoneProgress(kAnalyzerProgressDone);
        }

        m_currentTrack.reset();
    }
    DEBUG_ASSERT(readStopped());

    m_analyzers.clear();

    emitProgress(AnalyzerThreadState::Exit);
}

void AnalyzerThread::sendNextTrack(const TrackPointer& nextTrack) {
    DEBUG_ASSERT(!m_nextTrack.getValue());
    m_nextTrack.setValue(nextTrack);
    wake();
}

bool AnalyzerThread::readIdle() {
    DEBUG_ASSERT(!m_currentTrack);
    TrackPointer nextTrack = m_nextTrack.getValue();
    if (nextTrack) {
        m_nextTrack.setValue(TrackPointer());
        m_currentTrack = std::move(nextTrack);
        return false;
    } else {
        if (m_emittedState != AnalyzerThreadState::Idle) {
            // Only send the idle signal once when entering the
            // idle state from another state.
            emitProgress(AnalyzerThreadState::Idle);
        }
        return true;
    }
}

AnalyzerThread::AnalysisResult AnalyzerThread::analyzeAudioSource(
        const mixxx::AudioSourcePointer& audioSource) {
    DEBUG_ASSERT(m_currentTrack);

    mixxx::AudioSourceStereoProxy audioSourceProxy(
            audioSource,
            kAnalysisFramesPerBlock);
    DEBUG_ASSERT(audioSourceProxy.channelCount() == kAnalysisChannels);

    // Analysis starts now
    emitBusyProgress(kAnalyzerProgressNone);

    mixxx::IndexRange remainingFrames = audioSource->frameIndexRange();
    auto result = remainingFrames.empty() ? AnalysisResult::Complete : AnalysisResult::Pending;
    while (result == AnalysisResult::Pending) {
        whilePaused();
        if (readStopped()) {
            return AnalysisResult::Cancelled;
        }

        // 1st step: Decode next chunk of audio data
        const auto inputFrameIndexRange =
                remainingFrames.splitAndShrinkFront(
                        math_min(kAnalysisFramesPerBlock, remainingFrames.length()));
        DEBUG_ASSERT(!inputFrameIndexRange.empty());
        const auto readableSampleFrames =
                audioSourceProxy.readSampleFrames(
                        mixxx::WritableSampleFrames(
                                inputFrameIndexRange,
                                mixxx::SampleBuffer::WritableSlice(m_sampleBuffer)));

        whilePaused();
        if (readStopped()) {
            return AnalysisResult::Cancelled;
        }

        // 2nd: step: Analyze chunk of decoded audio data
        if (readableSampleFrames.frameLength() == kAnalysisFramesPerBlock) {
            // Complete chunk of audio samples has been read for analysis
            for (auto const& analyzer: m_analyzers) {
                analyzer->process(
                        readableSampleFrames.readableData(),
                        readableSampleFrames.readableLength());
            }
            if (remainingFrames.empty()) {
                result = AnalysisResult::Complete;
            }
        } else {
            // Partial chunk of audio samples has been read.
            // This should only happen at the end of an audio stream,
            // otherwise a decoding error must have occurred.
            if (remainingFrames.empty()) {
                result = AnalysisResult::Complete;
            } else {
                // EOF not reached -> Maybe a corrupt file?
                kLogger.warning()
                        << "Aborting analysis after failure to read sample data:"
                        << "expected frames =" << inputFrameIndexRange
                        << ", actual frames =" << readableSampleFrames.frameIndexRange();
                result = AnalysisResult::Partial;
            }
        }

        // Don't check again for paused/stopped and simply finish the
        // current iteration by emitting progress.

        // 3rd step: Update & emit progress
        const double frameProgress =
                double(audioSource->frameLength() - remainingFrames.length()) /
                double(audioSource->frameLength());
        const AnalyzerProgress progress =
                frameProgress *
                (kAnalyzerProgressFinalizing - kAnalyzerProgressNone);
        emitBusyProgress(progress);
    }

    return result;
}

void AnalyzerThread::emitBusyProgress(AnalyzerProgress busyProgress) {
    DEBUG_ASSERT(m_currentTrack);
    // The actual progress value is updated always even if the
    // following signal is inhibited (see below). The value is read
    // independently of the signal and should always reflect the
    // actual progress.
    m_analyzerProgress.setValue(busyProgress);
    // Signals are only sent with the specified maximum frequency
    // to avoid flooding the signal queue, which might impair the
    // responsiveness of the ui thread.
    const auto now = Clock::now();
    if ((m_emittedState == AnalyzerThreadState::Busy) &&
            (now < (m_lastBusyProgressEmittedAt + kBusyProgressInhibitDuration))) {
        // Don't update analyzer progress of the track
        // Don't emit progress signal
        return;
    }
    m_lastBusyProgressEmittedAt = now;
    emitProgress(AnalyzerThreadState::Busy);
}

void AnalyzerThread::emitDoneProgress(AnalyzerProgress doneProgress) {
    DEBUG_ASSERT(m_currentTrack);
    m_analyzerProgress.setValue(doneProgress);
    // Don't inhibit the final progress update!
    m_lastBusyProgressEmittedAt = Clock::now();
    emitProgress(AnalyzerThreadState::Done);
}

void AnalyzerThread::emitProgress(AnalyzerThreadState state) {
    m_emittedState = state;
    emit(progress(m_id, m_emittedState, m_currentTrack ? m_currentTrack->getId() : TrackId()));
}
