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

std::atomic<int> s_instanceCounter(0);

} // anonymous namespace

AnalyzerThread::AnalyzerThread(
        int id,
        mixxx::DbConnectionPoolPtr pDbConnectionPool,
        UserSettingsPointer pConfig)
        : m_id(id),
          m_pDbConnectionPool(std::move(pDbConnectionPool)),
          m_pConfig(std::move(pConfig)),
          m_run(true),
          m_sampleBuffer(kAnalysisSamplesPerBlock),
          m_emittedState(AnalyzerThreadState::Void),
          // The first signal should always be emitted
          m_lastBusyProgressEmittedAt(Clock::now() - kBusyProgressInhibitDuration) {
    m_nextTrack.setValue(TrackPointer());
    m_analyzerProgress.setValue(kAnalyzerProgressUnknown);
    // This type is registered multiple times although once would be sufficient
    qRegisterMetaType<AnalyzerThreadState>();
}

AnalyzerThread::~AnalyzerThread() {
    kLogger.debug() << "Destroying" << m_id;
    VERIFY_OR_DEBUG_ASSERT(isFinished()) {
        stop();
        // The following operation will block the host thread!
        wait();
    }
}

void AnalyzerThread::run() {
    if (!m_run.load()) {
        return;
    }

    const int instanceId = s_instanceCounter.fetch_add(1) + 1;
    QThread::currentThread()->setObjectName(QString("AnalyzerThread %1").arg(instanceId));

    kLogger.debug() << "Running" << m_id;

    exec();

    kLogger.debug() << "Exiting" << m_id;

    m_run.store(false);

    emitProgress(AnalyzerThreadState::Void);
}

void AnalyzerThread::exec() {
    // The thread-local database connection for waveform analysis
    // must not be closed before returning from this function.
    // Therefore the DbConnectionPooler is defined here independent
    // of whether a database connection will be opened or not.
    mixxx::DbConnectionPooler dbConnectionPooler;

    std::unique_ptr<AnalysisDao> pAnalysisDao;
    if (m_pConfig->getValue<bool>(ConfigKey("[Library]", "EnableWaveformGenerationWithAnalysis"), true)) {
        pAnalysisDao = std::make_unique<AnalysisDao>(m_pConfig);
        m_analyzers.push_back(std::make_unique<AnalyzerWaveform>(pAnalysisDao.get()));
    }
    m_analyzers.push_back(std::make_unique<AnalyzerGain>(m_pConfig));
    m_analyzers.push_back(std::make_unique<AnalyzerEbur128>(m_pConfig));
#ifdef __VAMP__
    m_analyzers.push_back(std::make_unique<AnalyzerBeats>(m_pConfig));
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

    while (m_run.load()) {
        TrackPointer track = recvNextTrack();
        if (!track) {
            break;
        }

        kLogger.debug() << "Analyzing" << track->getLocation();

        // Get the audio
        const auto audioSource =
                SoundSourceProxy(track).openAudioSource(openParams);
        if (!audioSource) {
            kLogger.warning()
                    << "Failed to open file for analyzing:"
                    << track->getLocation();
            emitDoneProgress(track, kAnalyzerProgressUnknown);
            continue;
        }

        bool processTrack = false;
        for (auto const& analyzer: m_analyzers) {
            // Make sure not to short-circuit initialize(...)
            if (analyzer->initialize(
                    track,
                    audioSource->sampleRate(),
                    audioSource->frameLength() * kAnalysisChannels)) {
                processTrack = true;
            }
        }

        if (processTrack) {
            const auto analysisResult = analyzeAudioSource(track, audioSource);
            DEBUG_ASSERT(analysisResult != AnalysisResult::Pending);
            if ((analysisResult == AnalysisResult::Complete) ||
                    (analysisResult == AnalysisResult::Partial)) {
                // The analysis has been finished, and is either complete without
                // any errors or partial if it has been aborted due to a corrupt
                // audio file. In both cases don't reanalyze tracks during this
                // session. A partial analysis would otherwise be repeated again
                // and again, because it is very unlikely that the error vanishes
                // suddenly.
                emitBusyProgress(track, kAnalyzerProgressFinalizing);
                // This takes around 3 sec on a Atom Netbook
                for (auto const& analyzer: m_analyzers) {
                    analyzer->finalize(track);
                }
                emitDoneProgress(track, kAnalyzerProgressDone);
            } else {
                for (auto const& analyzer: m_analyzers) {
                    analyzer->cleanup(track);
                }
                emitDoneProgress(track, kAnalyzerProgressUnknown);
            }
        } else {
            kLogger.debug() << "Skipping track analysis because no analyzer initialized.";
            emitDoneProgress(track, kAnalyzerProgressDone);
        }
    }
    DEBUG_ASSERT(!m_run.load());

    m_analyzers.clear();
}

void AnalyzerThread::stop() {
    kLogger.debug() << "Stopping" << m_id;
    m_run.store(false);
    m_idleWaitCond.notify_one();
}

void AnalyzerThread::sendNextTrack(const TrackPointer& nextTrack) {
    DEBUG_ASSERT(!m_nextTrack.getValue());
    m_nextTrack.setValue(nextTrack);
    m_idleWaitCond.notify_one();
}

TrackPointer AnalyzerThread::recvNextTrack() {
    std::unique_lock<std::mutex> locked(m_idleMutex);
    while (true) {
        if (!m_run.load()) {
            return TrackPointer();
        }
        TrackPointer nextTrack = m_nextTrack.getValue();
        if (nextTrack) {
            m_nextTrack.setValue(TrackPointer());
            return nextTrack;
        }
        kLogger.debug() << "Suspending" << m_id;
        if (m_emittedState != AnalyzerThreadState::Idle) {
            // Only send the idle signal once when entering the
            // idle state from another state.
            emitProgress(AnalyzerThreadState::Idle);
        }
        m_idleWaitCond.wait(locked) ;
        kLogger.debug() << "Resuming" << m_id;
    }
    return TrackPointer();
}

AnalyzerThread::AnalysisResult AnalyzerThread::analyzeAudioSource(
        const TrackPointer& track,
        const mixxx::AudioSourcePointer& audioSource) {

    mixxx::AudioSourceStereoProxy audioSourceProxy(
            audioSource,
            kAnalysisFramesPerBlock);
    DEBUG_ASSERT(audioSourceProxy.channelCount() == kAnalysisChannels);

    // Analysis starts now
    emitBusyProgress(track, kAnalyzerProgressNone);

    mixxx::IndexRange remainingFrames = audioSource->frameIndexRange();
    auto result = remainingFrames.empty() ? AnalysisResult::Complete : AnalysisResult::Pending;
    while (result == AnalysisResult::Pending) {
        if (!m_run.load()) {
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

        if (!m_run.load()) {
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

        if (!m_run.load()) {
            return AnalysisResult::Cancelled;
        }

        // 3rd step: Update & emit progress
        const double frameProgress =
                double(audioSource->frameLength() - remainingFrames.length()) /
                double(audioSource->frameLength());
        const AnalyzerProgress progress =
                frameProgress *
                (kAnalyzerProgressFinalizing - kAnalyzerProgressNone);
        emitBusyProgress(track, progress);
    }

    return result;
}

void AnalyzerThread::emitBusyProgress(const TrackPointer& track, AnalyzerProgress busyProgress) {
    DEBUG_ASSERT(track);
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
    emitProgress(AnalyzerThreadState::Busy, track);
}

void AnalyzerThread::emitDoneProgress(const TrackPointer& track, AnalyzerProgress doneProgress) {
    DEBUG_ASSERT(track);
    m_analyzerProgress.setValue(doneProgress);
    // Don't inhibit the final progress update!
    m_lastBusyProgressEmittedAt = Clock::now();
    emitProgress(AnalyzerThreadState::Done, track);
}

void AnalyzerThread::emitProgress(AnalyzerThreadState state, const TrackPointer& track) {
    m_emittedState = state;
    emit(progress(m_id, m_emittedState, track ? track->getId() : TrackId()));
}
