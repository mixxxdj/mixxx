#include "analyzer/analyzerqueue.h"

#ifdef __VAMP__
#include "analyzer/analyzerbeats.h"
#include "analyzer/analyzerkey.h"
#endif
#include "analyzer/analyzergain.h"
#include "analyzer/analyzerebur128.h"
#include "analyzer/analyzerwaveform.h"
#include "library/dao/analysisdao.h"
#include "engine/engine.h"
#include "mixer/playerinfo.h"
#include "sources/soundsourceproxy.h"
#include "sources/audiosourcestereoproxy.h"
#include "track/track.h"
#include "util/compatibility.h"
#include "util/db/dbconnectionpooler.h"
#include "util/db/dbconnectionpooled.h"
#include "util/event.h"
#include "util/timer.h"
#include "util/trace.h"
#include "util/logger.h"

namespace {

mixxx::Logger kLogger("AnalyzerQueue");

// Analysis is done in blocks.
// We need to use a smaller block size, because on Linux the AnalyzerQueue
// can starve the CPU of its resources, resulting in xruns. A block size
// of 4096 frames per block seems to do fine.
constexpr mixxx::AudioSignal::ChannelCount kAnalysisChannels = mixxx::kEngineChannelCount;
constexpr SINT kAnalysisFramesPerBlock = 4096;
const SINT kAnalysisSamplesPerBlock =
        kAnalysisFramesPerBlock * kAnalysisChannels;

// Maximum frequency of progress updates
constexpr int kProgressUpdateInhibitMillis = 100;

constexpr int kProgressStateEmpty   = 0;
constexpr int kProgressStateWriting = 1;
constexpr int kProgressStateReady   = 2;
constexpr int kProgressStateReading = 3;

QAtomicInt s_instanceCounter(0);

} // anonymous namespace

AnalyzerQueue::ProgressInfo::ProgressInfo()
    : m_state(kProgressStateEmpty),
      m_currentTrackProgress(kAnalysisProgressNone),
      m_queueSize(0) {
}

bool AnalyzerQueue::ProgressInfo::tryWrite(
        TracksWithProgress* pTracksWithProgress,
        TrackPointer pCurrentTrack,
        int currentTrackProgress,
        int queueSize) {
    DEBUG_ASSERT(pTracksWithProgress);
    bool wasEmpty = m_state.testAndSetAcquire(
            kProgressStateEmpty, kProgressStateWriting);
    bool progressChanged = false;
    if (wasEmpty || m_state.testAndSetAcquire(
            kProgressStateReady, kProgressStateWriting)) {
        DEBUG_ASSERT(m_state == kProgressStateWriting);
        // Keep all track references alive until the main thread releases
        // them by moving them into the progress info exchange object!
        if (!pTracksWithProgress->empty()) {
            if (m_tracksWithProgress.empty()) {
                pTracksWithProgress->swap(m_tracksWithProgress);
            } else {
                for (const auto trackProgress: *pTracksWithProgress) {
                    m_tracksWithProgress[trackProgress.first] = trackProgress.second;
                }
                pTracksWithProgress->insert(
                        m_tracksWithProgress.begin(),
                        m_tracksWithProgress.end());
            }
            pTracksWithProgress->clear();
            // Simply assume that something must have changed.
            // It is just too complicated to check for individual
            // changes.
            progressChanged = true;
        }
        if (pCurrentTrack) {
            const auto i = m_tracksWithProgress.find(pCurrentTrack);
            if (i == m_tracksWithProgress.end()) {
                m_tracksWithProgress[pCurrentTrack] = currentTrackProgress;
                progressChanged = true;
            } else if (i->second != currentTrackProgress) {
                i->second = currentTrackProgress;
                progressChanged = true;
            }
            m_currentTrackProgress = currentTrackProgress;
        }
        if (m_queueSize != queueSize) {
            m_queueSize = queueSize;
            progressChanged = true;
        }
        if (wasEmpty && !progressChanged) {
            // Still empty, nothing to do
            m_state = kProgressStateEmpty;
        } else {
            // Allow the main thread to consume progress info updates
            m_state = kProgressStateReady;
        }
    } else {
        // Ensure that track references are not dropped within the
        // analysis thread by accumulating progress updates until
        // the receiver is ready to consume them!
        if (pCurrentTrack) {
            (*pTracksWithProgress)[pCurrentTrack] = currentTrackProgress;
        }
    }
    return wasEmpty && progressChanged;
}

AnalyzerQueue::ProgressInfo::ReadScope::ReadScope(ProgressInfo* pProgressInfo)
    : m_pProgressInfo(nullptr) {
    DEBUG_ASSERT(pProgressInfo);
    // Defer updates from the analysis thread while the
    // current progress info is consumed.
    if (pProgressInfo->m_state.testAndSetAcquire(
            kProgressStateReady, kProgressStateReading)) {
        m_pProgressInfo = pProgressInfo;
    }
}

AnalyzerQueue::ProgressInfo::ReadScope::~ReadScope() {
    if (m_pProgressInfo) {
        DEBUG_ASSERT(m_pProgressInfo->m_state == kProgressStateReading);
        // Releasing all track reference here in the main thread might
        // trigger save actions. This is necessary to avoid that the
        // last reference is dropped within the analysis thread!
        m_pProgressInfo->m_tracksWithProgress.clear();
        // Finally allow the analysis thread to write progress info
        // updates again
        m_pProgressInfo->m_state = kProgressStateEmpty;
    }
}

AnalyzerQueue::AnalyzerQueue(
        mixxx::DbConnectionPoolPtr pDbConnectionPool,
        const UserSettingsPointer& pConfig,
        Mode mode)
        : m_pDbConnectionPool(std::move(pDbConnectionPool)),
          m_queueSize(0),
          m_queueModifiedFlag(false),
          m_exitPendingFlag(false),
          m_sampleBuffer(kAnalysisSamplesPerBlock) {

    if (mode != Mode::WithoutWaveform) {
        m_pAnalysisDao = std::make_unique<AnalysisDao>(pConfig);
        m_pAnalyzers.push_back(std::make_unique<AnalyzerWaveform>(m_pAnalysisDao.get()));
    }
    m_pAnalyzers.push_back(std::make_unique<AnalyzerGain>(pConfig));
    m_pAnalyzers.push_back(std::make_unique<AnalyzerEbur128>(pConfig));
#ifdef __VAMP__
    m_pAnalyzers.push_back(std::make_unique<AnalyzerBeats>(pConfig));
    m_pAnalyzers.push_back(std::make_unique<AnalyzerKey>(pConfig));
#endif
    kLogger.info() << "Activated" << m_pAnalyzers.size() << "analyzers";

    connect(this, SIGNAL(updateProgress()),
            this, SLOT(slotUpdateProgress()));

    start(QThread::LowPriority);
}

AnalyzerQueue::~AnalyzerQueue() {
    stop();
    // Wait until thread has actually stopped before proceeding
    wait();
}

// This is called from the AnalyzerQueue thread
bool AnalyzerQueue::isLoadedTrackQueued(TrackPointer pCurrentTrack) {
    const bool currentTrackLoaded =
            PlayerInfo::instance().isTrackLoaded(pCurrentTrack);
    bool prioritizeQueuedTrack = false;

    QMutexLocker locked(&m_qm);
    QMutableListIterator<TrackPointer> it(m_queuedTracks);
    while (it.hasNext()) {
        TrackPointer pQueuedTrack = it.next();
        DEBUG_ASSERT(pQueuedTrack);
        bool analysisNeeded = false;
        // Do not shortcut the following loop, because each
        // analyzer must be given a chance to load the analysis
        // results for this track!
        for (auto const& pAnalyzer: m_pAnalyzers) {
            if (!pAnalyzer->isDisabledOrLoadStoredSuccess(pQueuedTrack)) {
                analysisNeeded = true;
            }
        }
        if (analysisNeeded) {
            if (!currentTrackLoaded && !prioritizeQueuedTrack) {
                prioritizeQueuedTrack =
                        PlayerInfo::instance().isTrackLoaded(pQueuedTrack);
            }
        } else {
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Skipping re-analysis of file"
                        << pQueuedTrack->getLocation();
            }
            it.remove();
            m_pendingTracks.erase(pQueuedTrack);
            // Record progress for the next update cycle
            m_tracksWithProgress[pQueuedTrack] = kAnalysisProgressDone;
        }
    }

    // The analysis will be aborted if the currently analyzing
    // track is not loaded, but one or more tracks in the queue
    // are loaded and need to be prioritized.
    return prioritizeQueuedTrack;
}

// This is called from the AnalyzerQueue thread
// The returned track might be null if the analysis has been cancelled
TrackPointer AnalyzerQueue::dequeueNextBlocking() {
    QMutexLocker locked(&m_qm);

    Event::end("AnalyzerQueue process");
    while (m_queuedTracks.isEmpty()) {
        // One last try to deliver all collected updates before
        // suspending the analysis thread
        emitUpdateProgress();
        kLogger.debug() << "Suspending thread";
        m_qwait.wait(&m_qm);
        kLogger.debug() << "Resuming thread";

        if (m_exitPendingFlag) {
            return TrackPointer();
        }
    }
    Event::start("AnalyzerQueue process");

    const PlayerInfo& info = PlayerInfo::instance();
    QMutableListIterator<TrackPointer> it(m_queuedTracks);
    while (it.hasNext()) {
        TrackPointer pTrack = it.next();
        DEBUG_ASSERT(pTrack);
        // Prioritize tracks that are loaded.
        if (info.isTrackLoaded(pTrack)) {
            kLogger.debug() << "Prioritizing" << pTrack->getLocation();
            it.remove();
            return pTrack;
        }
    }

    // no prioritized track found, use head track
    DEBUG_ASSERT(!m_queuedTracks.isEmpty());
    return m_queuedTracks.dequeue();
}

// This is called from the AnalyzerQueue thread
AnalyzerQueue::AnalysisResult AnalyzerQueue::doAnalysis(
        TrackPointer pTrack,
        mixxx::AudioSourcePointer pAudioSource) {

    QTime progressUpdateInhibitTimer;
    progressUpdateInhibitTimer.start();

    mixxx::AudioSourceStereoProxy audioSourceProxy(
            pAudioSource,
            kAnalysisFramesPerBlock);
    DEBUG_ASSERT(audioSourceProxy.channelCount() == kAnalysisChannels);

    mixxx::IndexRange remainingFrames = pAudioSource->frameIndexRange();
    auto result = remainingFrames.empty() ? AnalysisResult::Complete : AnalysisResult::Pending;
    while (result == AnalysisResult::Pending) {
        ScopedTimer t("AnalyzerQueue::doAnalysis block");

        const auto inputFrameIndexRange =
                remainingFrames.splitAndShrinkFront(
                        math_min(kAnalysisFramesPerBlock, remainingFrames.length()));
        DEBUG_ASSERT(!inputFrameIndexRange.empty());
        const auto readableSampleFrames =
                audioSourceProxy.readSampleFrames(
                        mixxx::WritableSampleFrames(
                                inputFrameIndexRange,
                                mixxx::SampleBuffer::WritableSlice(m_sampleBuffer)));
        // To compare apples to apples, let's only look at blocks that are
        // the full block size.
        if (readableSampleFrames.frameLength() == kAnalysisFramesPerBlock) {
            // Complete analysis block of audio samples has been read.
            for (auto const& pAnalyzer: m_pAnalyzers) {
                pAnalyzer->process(
                        readableSampleFrames.readableData(),
                        readableSampleFrames.readableLength());
            }
            if (remainingFrames.empty()) {
                result = AnalysisResult::Complete;
            }
        } else {
            // Partial analysis block of audio samples has been read.
            // This should only happen at the end of an audio stream,
            // otherwise a decoding error must have occurred.
            if (remainingFrames.empty()) {
                result = AnalysisResult::Complete;
            } else {
                // EOF not reached -> Maybe a corrupt file?
                kLogger.warning()
                        << "Aborting analysis after failed to read sample data from"
                        << pTrack->getLocation()
                        << ": expected frames =" << inputFrameIndexRange
                        << ", actual frames =" << readableSampleFrames.frameIndexRange();
                result = AnalysisResult::Partial;
            }
        }

        // emit progress updates
        // During the doAnalysis function it goes only to kAnalysisProgressFinalizing
        // because the finalize functions will take also some time
        // fp div here prevents insane signed overflow
        const double frameProgress =
                double(pAudioSource->frameLength() - remainingFrames.length()) /
                double(pAudioSource->frameLength());
        int progressPromille = frameProgress * kAnalysisProgressFinalizing;

        // Since this is a background analysis queue, we should co-operatively
        // yield every now and then to try and reduce CPU contention. The
        // analyzer queue is CPU intensive so we want to get out of the way of
        // the audio callback thread.
        //QThread::yieldCurrentThread();
        //QThread::usleep(10);

        // has something new entered the queue?
        if (m_queueModifiedFlag.fetchAndStoreAcquire(false)) {
            if (isLoadedTrackQueued(pTrack)) {
                if (result == AnalysisResult::Pending) {
                    kLogger.debug() << "Interrupting analysis to give preference to a loaded track.";
                    result = AnalysisResult::Cancelled;
                }
            }
        }

        if ((m_progressInfo.m_currentTrackProgress != progressPromille) ||
                !m_tracksWithProgress.empty()) {
            if (progressUpdateInhibitTimer.elapsed() > kProgressUpdateInhibitMillis) {
                emitUpdateProgress(pTrack, progressPromille);
                progressUpdateInhibitTimer.start();
            }
        }

        if (m_exitPendingFlag) {
            result = AnalysisResult::Cancelled;
        }

        // Ignore blocks in which we decided to bail for stats purposes.
        if ((result != AnalysisResult::Pending) || (result != AnalysisResult::Complete)) {
            t.cancel();
        }
    }

    return result;
}

int AnalyzerQueue::resume() {
    m_queueModifiedFlag = true;
    QMutexLocker locked(&m_qm);
    m_qwait.wakeAll();
    return m_queuedTracks.size();
}

void AnalyzerQueue::stop() {
    m_exitPendingFlag = true;
    resume();
}

void AnalyzerQueue::run() {
    // If there are no analyzers, don't waste time running.
    if (m_pAnalyzers.empty()) {
        return;
    }

    const int instanceId = s_instanceCounter.fetchAndAddAcquire(1) + 1;
    QThread::currentThread()->setObjectName(QString("AnalyzerQueue %1").arg(instanceId));

    kLogger.debug() << "Entering thread";

    execThread();

    kLogger.debug() << "Exiting thread";
}

void AnalyzerQueue::execThread() {
    // The thread-local database connection for waveform analysis must not
    // be closed before returning from this function. Therefore the
    // DbConnectionPooler is defined at this outer function scope,
    // independent of whether a database connection will be opened
    // or not.
    mixxx::DbConnectionPooler dbConnectionPooler;
    // m_pAnalysisDao remains null if no analyzer needs database access.
    // Currently only waveform analyses makes use of it.
    if (m_pAnalysisDao) {
        dbConnectionPooler = mixxx::DbConnectionPooler(m_pDbConnectionPool); // move assignment
        if (!dbConnectionPooler.isPooling()) {
            kLogger.warning()
                    << "Failed to obtain database connection for analyzer queue thread";
            return;
        }
        // Obtain and use the newly created database connection within this thread
        QSqlDatabase dbConnection = mixxx::DbConnectionPooled(m_pDbConnectionPool);
        DEBUG_ASSERT(dbConnection.isOpen());
        m_pAnalysisDao->initialize(dbConnection);
    }

    while (!m_exitPendingFlag) {
        TrackPointer nextTrack = dequeueNextBlocking();
        if (m_exitPendingFlag) {
            emptyCheck();
            break;
        }

        DEBUG_ASSERT(nextTrack);
        kLogger.debug() << "Analyzing" << nextTrack->getTitle() << nextTrack->getLocation();

        Trace trace("AnalyzerQueue analyzing track");

        // Get the audio
        mixxx::AudioSource::OpenParams openParams;
        openParams.setChannelCount(kAnalysisChannels);
        auto pAudioSource = SoundSourceProxy(nextTrack).openAudioSource(openParams);
        if (!pAudioSource) {
            kLogger.warning()
                    << "Failed to open file for analyzing:"
                    << nextTrack->getLocation();
            emptyCheck();
            continue;
        }

        bool processTrack = false;
        for (auto const& pAnalyzer: m_pAnalyzers) {
            // Make sure not to short-circuit initialize(...)
            if (pAnalyzer->initialize(
                    nextTrack,
                    pAudioSource->sampleRate(),
                    pAudioSource->frameLength() * kAnalysisChannels)) {
                processTrack = true;
            }
        }

        updateSize();

        if (processTrack) {
            emitUpdateProgress(nextTrack, kAnalysisProgressNone);
            const auto analysisResult = doAnalysis(nextTrack, pAudioSource);
            DEBUG_ASSERT(analysisResult != AnalysisResult::Pending);
            if ((analysisResult == AnalysisResult::Complete) ||
                    (analysisResult == AnalysisResult::Partial)) {
                // The analysis has been finished, and is either complete without
                // any errors or partial if it has been aborted due to a corrupt
                // audio file. In both cases don't reanalyze tracks during this
                // session. A partial analysis would otherwise be repeated again
                // and again, because it is very unlikely that the error vanishes
                // suddenly.
                emitUpdateProgress(nextTrack, kAnalysisProgressFinalizing);
                // This takes around 3 sec on a Atom Netbook
                for (auto const& pAnalyzer: m_pAnalyzers) {
                    pAnalyzer->finalize(nextTrack);
                }
                emit(trackDone(nextTrack));
                m_pendingTracks.erase(nextTrack);
                emitUpdateProgress(std::move(nextTrack), kAnalysisProgressDone);
            } else {
                for (auto const& pAnalyzer: m_pAnalyzers) {
                    pAnalyzer->cleanup(nextTrack);
                }
                // still pending -> re-enqueue
                enqueueTrack(nextTrack);
                emitUpdateProgress(std::move(nextTrack), kAnalysisProgressNone);
            }
        } else {
            kLogger.debug() << "Skipping track analysis because no analyzer initialized.";
            m_pendingTracks.erase(nextTrack);
            emitUpdateProgress(std::move(nextTrack), kAnalysisProgressDone);
        }
        // All references to the track object within the analysis thread
        // should have been released to avoid exporting metadata or updating
        // the database within the low-prio analysis thread!
        DEBUG_ASSERT(!nextTrack);
        emptyCheck();
    }

    if (m_pAnalysisDao) {
        // Invalidate reference to the thread-local database connection
        // that will be closed soon. Not necessary, just in case ;)
        m_pAnalysisDao->initialize(QSqlDatabase());
    }

    emit(queueEmpty()); // emit in case of exit;
}

void AnalyzerQueue::emptyCheck() {
    updateSize();
    if (m_queueSize == 0) {
        emit(queueEmpty()); // emit asynchrony for no deadlock
    }
}

void AnalyzerQueue::updateSize() {
    QMutexLocker locked(&m_qm);
    m_queueSize = m_queuedTracks.size();
}

// This is called from the AnalyzerQueue thread
void AnalyzerQueue::emitUpdateProgress(TrackPointer pTrack, int progress) {
    if (!m_exitPendingFlag) {
        if (m_progressInfo.tryWrite(&m_tracksWithProgress, std::move(pTrack), progress, m_queueSize)) {
            emit(updateProgress());
        }
    }
}

//slot
void AnalyzerQueue::slotUpdateProgress() {
    const auto readScope = m_progressInfo.read();
    if (readScope) {
        // TODO(XXX): Bulk updates of many track objects will
        // limit the responsiveness of the UI or may cause freezing.
        bool oneOrMoreTracksFinished = false;
        for (const auto trackWithProgress: readScope.tracksWithProgress()) {
            trackWithProgress.first->setAnalysisProgress(trackWithProgress.second);
            if (trackWithProgress.second == kAnalysisProgressDone) {
                oneOrMoreTracksFinished = true;
            }
        }
        DEBUG_ASSERT(oneOrMoreTracksFinished ||
                (readScope.currentTrackProgress() != kAnalysisProgressDone));
        int trackProgressPercent = readScope.currentTrackProgress() / 10;
        emit(trackProgress(trackProgressPercent));
        if (oneOrMoreTracksFinished) {
            emit(trackFinished(readScope.queueSize()));
        }
    }
}

void AnalyzerQueue::slotAnalyseTrack(TrackPointer pTrack) {
    // This slot is called from the decks and and samplers when the track was loaded.
    if (enqueueTrack(pTrack)) {
        resume();
    }
}

// This is called from the GUI and from the AnalyzerQueue thread
bool AnalyzerQueue::enqueueTrack(TrackPointer pTrack) {
    VERIFY_OR_DEBUG_ASSERT(pTrack) {
        return false;
    }

    QMutexLocker locked(&m_qm);
    if (m_pendingTracks.insert(pTrack).second) {
        if (PlayerInfo::instance().isTrackLoaded(pTrack)) {
            // Prioritize track
            m_queuedTracks.prepend(pTrack);
        } else {
            m_queuedTracks.enqueue(pTrack);
        }
        // Don't wake up the paused thread now to avoid race conditions
        // if multiple threads are added in a row. The caller is
        // responsible to finish the enqueuing of tracks with resume().
        return true;
    } else {
        if (kLogger.debugEnabled()) {
            kLogger.debug()
                    << "Analysis of track is already pending"
                    << pTrack->getLocation();
        }
        return false;
    }
}
