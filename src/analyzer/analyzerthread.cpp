#include "analyzer/analyzerthread.h"

#include <mutex>

#include "analyzer/analyzerbeats.h"
#include "analyzer/analyzerebur128.h"
#include "analyzer/analyzergain.h"
#include "analyzer/analyzerkey.h"
#include "analyzer/analyzersilence.h"
#include "analyzer/analyzerwaveform.h"
#include "analyzer/constants.h"
#include "engine/engine.h"
#include "library/dao/analysisdao.h"
#include "sources/audiosourcestereoproxy.h"
#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "util/db/dbconnectionpooled.h"
#include "util/db/dbconnectionpooler.h"
#include "util/logger.h"
#include "util/timer.h"

namespace {

mixxx::Logger kLogger("AnalyzerThread");

// NOTE(uklotzde, 2018-11-23): The parameterization for the analyzers
// has not been touched while transforming the code from single- to
// multi-threaded processing! Feel free to adjust this if justified.

/// TODO(XXX): Use the vsync timer for the purpose of sending updates
// to the UI thread with a limited rate??

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
        : Pointer(nullptr, [](AnalyzerThread*) {}) {
}

//static
AnalyzerThread::Pointer AnalyzerThread::createInstance(
        int id,
        mixxx::DbConnectionPoolPtr dbConnectionPool,
        UserSettingsPointer pConfig,
        AnalyzerModeFlags modeFlags) {
    return Pointer(new AnalyzerThread(
                           id,
                           dbConnectionPool,
                           pConfig,
                           modeFlags),
            deleteAnalyzerThread);
}

AnalyzerThread::AnalyzerThread(
        int id,
        mixxx::DbConnectionPoolPtr dbConnectionPool,
        UserSettingsPointer pConfig,
        AnalyzerModeFlags modeFlags)
        : WorkerThread(
            QString("AnalyzerThread %1").arg(id),
            (modeFlags & AnalyzerModeFlags::LowPriority ? QThread::LowPriority : QThread::InheritPriority)),
          m_id(id),
          m_dbConnectionPool(std::move(dbConnectionPool)),
          m_pConfig(pConfig),
          m_modeFlags(modeFlags),
          m_nextTrack(2), // minimum capacity
          m_sampleBuffer(mixxx::kAnalysisSamplesPerChunk),
          m_emittedState(AnalyzerThreadState::Void) {
    std::call_once(registerMetaTypesOnceFlag, registerMetaTypesOnce);
}

void AnalyzerThread::doRun() {
    std::unique_ptr<AnalysisDao> pAnalysisDao;
    // The thread-local database connection  must not be closed
    // before returning from this function.
    mixxx::DbConnectionPooler dbConnectionPooler;

    if (m_modeFlags & AnalyzerModeFlags::WithWaveform) {
        dbConnectionPooler = mixxx::DbConnectionPooler(m_dbConnectionPool); // move assignment
        if (!dbConnectionPooler.isPooling()) {
            kLogger.warning()
                    << "Failed to obtain database connection for analyzer thread";
            return;
        }
        QSqlDatabase dbConnection = mixxx::DbConnectionPooled(m_dbConnectionPool);
        m_analyzers.push_back(AnalyzerWithState(std::make_unique<AnalyzerWaveform>(m_pConfig, dbConnection)));
    }
    if (AnalyzerGain::isEnabled(ReplayGainSettings(m_pConfig))) {
        m_analyzers.push_back(AnalyzerWithState(std::make_unique<AnalyzerGain>(m_pConfig)));
    }
    if (AnalyzerEbur128::isEnabled(ReplayGainSettings(m_pConfig))) {
        m_analyzers.push_back(AnalyzerWithState(std::make_unique<AnalyzerEbur128>(m_pConfig)));
    }
    // BPM detection might be disabled in the config, but can be overridden
    // and enabled by explicitly setting the mode flag.
    const bool enforceBpmDetection = (m_modeFlags & AnalyzerModeFlags::WithBeats) != 0;
    m_analyzers.push_back(AnalyzerWithState(std::make_unique<AnalyzerBeats>(m_pConfig, enforceBpmDetection)));
    m_analyzers.push_back(AnalyzerWithState(std::make_unique<AnalyzerKey>(m_pConfig)));
    m_analyzers.push_back(AnalyzerWithState(std::make_unique<AnalyzerSilence>(m_pConfig)));
    DEBUG_ASSERT(!m_analyzers.empty());
    kLogger.debug() << "Activated" << m_analyzers.size() << "analyzers";

    m_lastBusyProgressEmittedTimer.start();

    mixxx::AudioSource::OpenParams openParams;
    openParams.setChannelCount(mixxx::kAnalysisChannels);

    while (awaitWorkItemsFetched()) {
        DEBUG_ASSERT(m_currentTrack);
        kLogger.debug() << "Analyzing" << m_currentTrack->getFileInfo();

        // Get the audio
        const auto audioSource =
                SoundSourceProxy(m_currentTrack).openAudioSource(openParams);
        if (!audioSource) {
            kLogger.warning()
                    << "Failed to open file for analyzing:"
                    << m_currentTrack->getFileInfo();
            emitDoneProgress(kAnalyzerProgressUnknown);
            continue;
        }

        bool processTrack = false;
        for (auto&& analyzer : m_analyzers) {
            // Make sure not to short-circuit initialize(...)
            if (analyzer.initialize(
                        m_currentTrack,
                        audioSource->getSignalInfo().getSampleRate(),
                        audioSource->frameLength() * mixxx::kAnalysisChannels)) {
                processTrack = true;
            }
        }

        if (processTrack) {
            const auto analysisResult = analyzeAudioSource(audioSource);
            DEBUG_ASSERT(analysisResult != AnalysisResult::Pending);
            if (analysisResult == AnalysisResult::Finished) {
                // The analysis has been finished, and is either complete without
                // any errors or partial if it has been aborted due to a corrupt
                // audio file. In both cases don't reanalyze tracks during this
                // session. A partial analysis would otherwise be repeated again
                // and again, because it is very unlikely that the error vanishes
                // suddenly.
                emitBusyProgress(kAnalyzerProgressFinalizing);
                // This takes around 3 sec on a Atom Netbook
                for (auto&& analyzer : m_analyzers) {
                    analyzer.finish(m_currentTrack);
                }
                emitDoneProgress(kAnalyzerProgressDone);
            } else {
                for (auto&& analyzer : m_analyzers) {
                    analyzer.cancel();
                }
                emitDoneProgress(kAnalyzerProgressUnknown);
            }
        } else {
            kLogger.debug() << "Skipping track analysis because no analyzer initialized.";
            emitDoneProgress(kAnalyzerProgressDone);
        }
    }
    DEBUG_ASSERT(!m_currentTrack);
    DEBUG_ASSERT(isStopping());

    m_analyzers.clear();

    kLogger.debug() << "Exiting worker thread";
    emitProgress(AnalyzerThreadState::Exit);
}

bool AnalyzerThread::submitNextTrack(TrackPointer nextTrack) {
    DEBUG_ASSERT(nextTrack);
    kLogger.debug()
            << "Enqueueing next track"
            << nextTrack->getId();
    if (m_nextTrack.try_emplace(std::move(nextTrack))) {
        // Ensure that the submitted track gets processed eventually
        // by waking the worker thread up after adding a new task to
        // its back queue! Otherwise the thread might not notice if
        // it is currently idle and has fallen asleep.
        wake();
        return true;
    }
    return false;
}

WorkerThread::TryFetchWorkItemsResult AnalyzerThread::tryFetchWorkItems() {
    DEBUG_ASSERT(!m_currentTrack);
    TrackPointer* pFront = m_nextTrack.front();
    if (pFront) {
        m_currentTrack = *pFront;
        m_nextTrack.pop();
        kLogger.debug()
                << "Dequeued next track"
                << m_currentTrack->getId();
        return TryFetchWorkItemsResult::Ready;
    } else {
        emitProgress(AnalyzerThreadState::Idle);
        return TryFetchWorkItemsResult::Idle;
    }
}

AnalyzerThread::AnalysisResult AnalyzerThread::analyzeAudioSource(
        const mixxx::AudioSourcePointer& audioSource) {
    DEBUG_ASSERT(m_currentTrack);

    mixxx::AudioSourceStereoProxy audioSourceProxy(
            audioSource,
            mixxx::kAnalysisFramesPerChunk);
    DEBUG_ASSERT(
            audioSourceProxy.getSignalInfo().getChannelCount() ==
            mixxx::kAnalysisChannels);

    // Analysis starts now
    emitBusyProgress(kAnalyzerProgressNone);

    mixxx::IndexRange remainingFrameRange = audioSource->frameIndexRange();
    while (!remainingFrameRange.empty()) {
        sleepWhileSuspended();
        if (isStopping()) {
            return AnalysisResult::Cancelled;
        }

        // 1st step: Decode next chunk of audio data

        // Split the range for the next chunk from the remaining (= to-be-analyzed) frames
        auto chunkFrameRange =
                remainingFrameRange.splitAndShrinkFront(
                        math_min(mixxx::kAnalysisFramesPerChunk, remainingFrameRange.length()));
        DEBUG_ASSERT(!chunkFrameRange.empty());

        // Request the next chunk of audio data
        const auto readableSampleFrames =
                audioSourceProxy.readSampleFrames(
                        mixxx::WritableSampleFrames(
                                chunkFrameRange,
                                mixxx::SampleBuffer::WritableSlice(m_sampleBuffer)));
        // The returned range fits into the requested range
        DEBUG_ASSERT(readableSampleFrames.frameIndexRange() <= chunkFrameRange);

        // Sometimes the duration of the audio source is inaccurate and adjusted
        // while reading. We need to adjust all frame ranges to reflect this new
        // situation by restoring all invariants and consistency requirements!

        // Shrink the original range of the current chunks to the actual available
        // range.
        chunkFrameRange = intersect(chunkFrameRange, audioSourceProxy.frameIndexRange());
        // The audio data that has just been read should still fit into the adjusted
        // chunk range.
        DEBUG_ASSERT(readableSampleFrames.frameIndexRange() <= chunkFrameRange);

        // We also need to adjust the remaining frame range for the next requests.
        remainingFrameRange = intersect(remainingFrameRange, audioSourceProxy.frameIndexRange());
        // Currently the range will never grow, but lets also account for this case
        // that might become relevant in the future.
        VERIFY_OR_DEBUG_ASSERT(remainingFrameRange.empty() ||
                remainingFrameRange.end() == audioSourceProxy.frameIndexRange().end()) {
            if (chunkFrameRange.length() < mixxx::kAnalysisFramesPerChunk) {
                // If we have read an incomplete chunk while the range has grown
                // we need to discard the read results and re-read the current
                // chunk!
                remainingFrameRange = span(remainingFrameRange, chunkFrameRange);
                continue;
            }
            DEBUG_ASSERT(remainingFrameRange.end() < audioSourceProxy.frameIndexRange().end());
            kLogger.warning()
                    << "Unexpected growth of the audio source while reading"
                    << mixxx::IndexRange::forward(
                            remainingFrameRange.end(), audioSourceProxy.frameIndexRange().end());
            remainingFrameRange.growBack(
                    audioSourceProxy.frameIndexRange().end() - remainingFrameRange.end());
        }

        sleepWhileSuspended();
        if (isStopping()) {
            return AnalysisResult::Cancelled;
        }

        // 2nd: step: Analyze chunk of decoded audio data
        if (!readableSampleFrames.frameIndexRange().empty()) {
            for (auto&& analyzer : m_analyzers) {
                analyzer.processSamples(
                        readableSampleFrames.readableData(),
                        readableSampleFrames.readableLength());
            }
        }

        // Don't check again for paused/stopped again and simply finish
        // the current iteration by emitting progress.

        // 3rd step: Update & emit progress
        if (audioSource->frameLength() > 0) {
            const double frameProgress =
                    double(audioSource->frameLength() - remainingFrameRange.length()) /
                    double(audioSource->frameLength());
            // math_min is required to compensate rounding errors
            const AnalyzerProgress progress =
                    math_min(kAnalyzerProgressFinalizing,
                            frameProgress *
                                    (kAnalyzerProgressFinalizing - kAnalyzerProgressNone));
            DEBUG_ASSERT(progress > kAnalyzerProgressNone);
            emitBusyProgress(progress);
        } else {
            // Unreadable audio source
            DEBUG_ASSERT(remainingFrameRange.empty());
            emitBusyProgress(kAnalyzerProgressUnknown);
        }
    }

    return AnalysisResult::Finished;
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
    m_currentTrack->analysisFinished();
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
