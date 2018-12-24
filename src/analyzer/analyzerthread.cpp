#include "analyzer/analyzerthread.h"

#include <mutex>

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

// NOTE(uklotzde, 2018-11-23): The parameterization for the analyzers
// has not been touched while transforming the code from single- to
// multi-threaded processing! Feel free to adjust this if justified.

// Analysis is done in blocks to avoid dynamic allocation of memory
// depending on the track length. A block size of 4096 frames per block
// seems to do fine. Signal processing during analysis uses the same,
// fixed number of channels like the engine does, usually 2 = stereo.
constexpr mixxx::AudioSignal::ChannelCount kAnalysisChannels = mixxx::kEngineChannelCount;
constexpr SINT kAnalysisFramesPerBlock = 4096;
const SINT kAnalysisSamplesPerBlock =
        kAnalysisFramesPerBlock * kAnalysisChannels;

// Maximum frequency of progress updates while busy. A value of 60 ms
// results in ~17 progress updates per second which is sufficient for
// continuous feedback.
const mixxx::Duration kBusyProgressInhibitDuration = mixxx::Duration::fromMillis(60);

void deleteAnalyzerThread(AnalyzerThread* plainPtr) {
    if (plainPtr) {
        plainPtr->deleteAfterFinished();
    }
}

std::once_flag registerMetaTypesOnceFlag;

void registerMetaTypesOnce() {
    qRegisterMetaType<AnalyzerThreadState>();
    // AnalyzerProgress is just an alias/typedef and must be registered explicitly
    // by name!
    qRegisterMetaType<AnalyzerProgress>("AnalyzerProgress");
}

} // anonymous namespace

AnalyzerThread::NullPointer::NullPointer()
    : Pointer(nullptr, [](AnalyzerThread*){}) {
}

//static
AnalyzerThread::Pointer AnalyzerThread::createInstance(
        int id,
        mixxx::DbConnectionPoolPtr dbConnectionPool,
        UserSettingsPointer pConfig,
        AnalyzerMode mode) {
    return Pointer(new AnalyzerThread(
            id,
            dbConnectionPool,
            pConfig,
            mode),
            deleteAnalyzerThread);
}

AnalyzerThread::AnalyzerThread(
        int id,
        mixxx::DbConnectionPoolPtr dbConnectionPool,
        UserSettingsPointer pConfig,
        AnalyzerMode mode)
        : WorkerThread(QString("AnalyzerThread %1").arg(id)),
          m_id(id),
          m_dbConnectionPool(std::move(dbConnectionPool)),
          m_pConfig(std::move(pConfig)),
          m_mode(mode),
          m_sampleBuffer(kAnalysisSamplesPerBlock),
          m_emittedState(AnalyzerThreadState::Void) {
    std::call_once(registerMetaTypesOnceFlag, registerMetaTypesOnce);
    m_lastBusyProgressEmittedTimer.start();
}

void AnalyzerThread::doRun() {
    std::unique_ptr<AnalysisDao> pAnalysisDao;
    if (m_mode != AnalyzerMode::WithBeatsWithoutWaveform) {
        pAnalysisDao = std::make_unique<AnalysisDao>(m_pConfig);
        m_analyzers.push_back(std::make_unique<AnalyzerWaveform>(pAnalysisDao.get()));
    }
    if (AnalyzerGain::isEnabled(ReplayGainSettings(m_pConfig))) {
        m_analyzers.push_back(std::make_unique<AnalyzerGain>(m_pConfig));
    }
    if (AnalyzerEbur128::isEnabled(ReplayGainSettings(m_pConfig))) {
        m_analyzers.push_back(std::make_unique<AnalyzerEbur128>(m_pConfig));
    }
#ifdef __VAMP__
    const bool enforceBpmDetection = m_mode != AnalyzerMode::Default;
    m_analyzers.push_back(std::make_unique<AnalyzerBeats>(m_pConfig, enforceBpmDetection));
    m_analyzers.push_back(std::make_unique<AnalyzerKey>(m_pConfig));
#endif
    DEBUG_ASSERT(!m_analyzers.empty());
    kLogger.debug() << "Activated" << m_analyzers.size() << "analyzers";

    // This thread-local database connection for pAnalysisDao
    // must not be closed before returning from this function.
    // Therefore the DbConnectionPooler is defined outside of
    // the conditional if block.
    mixxx::DbConnectionPooler dbConnectionPooler;
    if (pAnalysisDao) {
        dbConnectionPooler = mixxx::DbConnectionPooler(m_dbConnectionPool); // move assignment
        if (!dbConnectionPooler.isPooling()) {
            kLogger.warning()
                    << "Failed to obtain database connection for analyzer queue thread";
            return;
        }
        // Obtain and use the newly created database connection within this thread
        QSqlDatabase dbConnection = mixxx::DbConnectionPooled(m_dbConnectionPool);
        DEBUG_ASSERT(dbConnection.isOpen());
        pAnalysisDao->initialize(dbConnection);
    }

    mixxx::AudioSource::OpenParams openParams;
    openParams.setChannelCount(kAnalysisChannels);

    while (waitUntilWorkItemsFetched()) {
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
    }
    DEBUG_ASSERT(isStopping());

    m_analyzers.clear();

    emitProgress(AnalyzerThreadState::Exit);
}

bool AnalyzerThread::submitNextTrack(TrackPointer nextTrack) {
    DEBUG_ASSERT(nextTrack);
    return m_nextTrack.enqueue(std::move(nextTrack));
}

WorkerThread::FetchWorkResult AnalyzerThread::tryFetchWorkItems() {
    DEBUG_ASSERT(!m_currentTrack);
    if (m_nextTrack.dequeue(&m_currentTrack)) {
        DEBUG_ASSERT(m_currentTrack);
        return FetchWorkResult::Ready;
    } else {
        if (m_emittedState != AnalyzerThreadState::Idle) {
            // Only send the idle signal once when entering the
            // idle state from another state.
            emitProgress(AnalyzerThreadState::Idle);
        }
        return FetchWorkResult::Idle;
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
        sleepWhileSuspended();
        if (isStopping()) {
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

        sleepWhileSuspended();
        if (isStopping()) {
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
    if ((m_emittedState == AnalyzerThreadState::Busy) &&
            (m_lastBusyProgressEmittedTimer.elapsed() < kBusyProgressInhibitDuration)) {
        // Don't emit progress signal while still busy and the
        // previous progress signal has been emitted just recently.
        // This should keep the host thread responsive and prevents
        // to overwhelm it with too frequent progress signals.
        return;
    }
    m_lastBusyProgressEmittedTimer.restart();
    emitProgress(AnalyzerThreadState::Busy, m_currentTrack->getId(), busyProgress);
    DEBUG_ASSERT(m_emittedState == AnalyzerThreadState::Busy);
}

void AnalyzerThread::emitDoneProgress(AnalyzerProgress doneProgress) {
    DEBUG_ASSERT(m_currentTrack);
    // Release all references of the track before emitting the signal
    // to ensure that the last reference is not dropped in this worker
    // thread that might trigger database actions! The TrackAnalysisScheduler
    // must store a TrackPointer until receiving the Done signal.
    TrackId trackId = m_currentTrack->getId();
    m_currentTrack.reset();
    emitProgress(AnalyzerThreadState::Done, trackId, doneProgress);
}

void AnalyzerThread::emitProgress(AnalyzerThreadState state) {
    DEBUG_ASSERT(!m_currentTrack);
    emitProgress(state, TrackId(), kAnalyzerProgressUnknown);
}

void AnalyzerThread::emitProgress(AnalyzerThreadState state, TrackId trackId, AnalyzerProgress trackProgress) {
    DEBUG_ASSERT(!m_currentTrack || (state == AnalyzerThreadState::Busy));
    DEBUG_ASSERT(!m_currentTrack || (m_currentTrack->getId() == trackId));
    DEBUG_ASSERT(trackId.isValid() || (trackProgress == kAnalyzerProgressUnknown));
    m_emittedState = state;
    emit progress(m_id, m_emittedState, trackId, trackProgress);
}
