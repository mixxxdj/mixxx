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

// Measured in 0.1%,
// 0 for no progress during finalize
// 1 to display the text "finalizing"
// 100 for 10% step after finalize
#define FINALIZE_PROMILLE 1

namespace {

mixxx::Logger kLogger("AnalyzerQueue");

// Analysis is done in blocks.
// We need to use a smaller block size, because on Linux the AnalyzerQueue
// can starve the CPU of its resources, resulting in xruns. A block size
// of 4096 frames per block seems to do fine.
const mixxx::AudioSignal::ChannelCount kAnalysisChannels(mixxx::kEngineChannelCount);
const SINT kAnalysisFramesPerBlock = 4096;
const SINT kAnalysisSamplesPerBlock =
        kAnalysisFramesPerBlock * kAnalysisChannels;

QAtomicInt s_instanceCounter(0);

} // anonymous namespace

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

    connect(this, SIGNAL(updateProgress()),
            this, SLOT(slotUpdateProgress()));

    start(QThread::LowPriority);
}

AnalyzerQueue::~AnalyzerQueue() {
    stop();
    m_progressInfo.sema.release();
    wait(); //Wait until thread has actually stopped before proceeding.
}

// This is called from the AnalyzerQueue thread
bool AnalyzerQueue::isLoadedTrackWaiting(TrackPointer pAnalyzingTrack) {
    bool anyQueuedTrackLoaded = false;
    QList<TrackPointer> waitingTracks;
    QList<TrackPointer> finishedTracks;

    QMutexLocker locked(&m_qm);
    QMutableListIterator<TrackPointer> it(m_queuedTracks);
    while (it.hasNext()) {
        TrackPointer pTrack = it.next();
        DEBUG_ASSERT(pTrack);
        if (!anyQueuedTrackLoaded) {
            anyQueuedTrackLoaded = PlayerInfo::instance().isTrackLoaded(pTrack);
        }
        // try to load waveforms for all new tracks first
        // and remove them from queue if already analysed
        // This avoids waiting for a running analysis for those tracks.
        int progress = pTrack->getAnalyzerProgress();
        if (progress < 0) {
            // Load stored analysis
            bool processTrack = false;
            for (auto const& pAnalyzer: m_pAnalyzers) {
                if (!pAnalyzer->isDisabledOrLoadStoredSuccess(pTrack)) {
                    processTrack = true;
                }
            }
            if (processTrack) {
                waitingTracks.append(pTrack);
            } else {
                kLogger.debug()
                        << "Skipping analysis of file"
                        << pTrack->getLocation();
                finishedTracks.append(pTrack);
                it.remove();
            }
        } else if (progress == 1000) {
            it.remove();
        }
    }
    locked.unlock();

    // update progress after unlock to avoid a deadlock
    foreach (TrackPointer pTrack, waitingTracks) {
        emitUpdateProgress(pTrack, 0);
    }
    foreach (TrackPointer pTrack, finishedTracks) {
        emitUpdateProgress(pTrack, 1000);
    }

    // The analysis will be aborted if the currently analyzing
    // track is not loaded, but one or more tracks in the queue
    // are loaded and need to be prioritized.
    return anyQueuedTrackLoaded &&
            !PlayerInfo::instance().isTrackLoaded(pAnalyzingTrack);
}

// This is called from the AnalyzerQueue thread
// The returned track might be null if the analysis has been cancelled
TrackPointer AnalyzerQueue::dequeueNextBlocking() {
    QMutexLocker locked(&m_qm);

    Event::end("AnalyzerQueue process");
    while (m_queuedTracks.isEmpty()) {
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
    progressUpdateInhibitTimer.start(); // Inhibit Updates for 60 milliseconds

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
        // During the doAnalysis function it goes only to 100% - FINALIZE_PERCENT
        // because the finalize functions will take also some time
        // fp div here prevents insane signed overflow
        const double frameProgress =
                double(pAudioSource->frameLength() - remainingFrames.length()) /
                double(pAudioSource->frameLength());
        int progressPromille = frameProgress * (1000 - FINALIZE_PROMILLE);

        if (m_progressInfo.track_progress != progressPromille) {
            if (progressUpdateInhibitTimer.elapsed() > 60) {
                // Inhibit Updates for 60 milliseconds
                emitUpdateProgress(pTrack, progressPromille);
                progressUpdateInhibitTimer.start();
            }
        }

        // Since this is a background analysis queue, we should co-operatively
        // yield every now and then to try and reduce CPU contention. The
        // analyzer queue is CPU intensive so we want to get out of the way of
        // the audio callback thread.
        //QThread::yieldCurrentThread();
        //QThread::usleep(10);

        // has something new entered the queue?
        if (m_queueModifiedFlag.fetchAndStoreAcquire(false)) {
            if (isLoadedTrackWaiting(pTrack)) {
                if (result == AnalysisResult::Pending) {
                    kLogger.debug() << "Interrupting analysis to give preference to a loaded track.";
                    result = AnalysisResult::Cancelled;
                }
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

    m_progressInfo.current_track.reset();
    m_progressInfo.track_progress = 0;
    m_progressInfo.queue_size = 0;
    m_progressInfo.sema.release(); // Initialize with one

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
            emitUpdateProgress(nextTrack, 0);
            const auto analysisResult = doAnalysis(nextTrack, pAudioSource);
            DEBUG_ASSERT(analysisResult != AnalysisResult::Pending);
            if ((analysisResult == AnalysisResult::Complete) ||
                    (analysisResult == AnalysisResult::Partial)) {
                // Don't try to reanalyze tracks during this session if the
                // analysis failed even if only partial results are available!
                // 100% - FINALIZE_PERCENT finished
                emitUpdateProgress(nextTrack, 1000 - FINALIZE_PROMILLE);
                // This takes around 3 sec on a Atom Netbook
                for (auto const& pAnalyzer: m_pAnalyzers) {
                    pAnalyzer->finalize(nextTrack);
                }
                emit(trackDone(nextTrack));
                emitUpdateProgress(nextTrack, 1000); // 100%
            } else {
                for (auto const& pAnalyzer: m_pAnalyzers) {
                    pAnalyzer->cleanup(nextTrack);
                }
                enqueueTrack(nextTrack);
                emitUpdateProgress(nextTrack, 0);
            }
        } else {
            emitUpdateProgress(nextTrack, 1000); // 100%
            kLogger.debug() << "Skipping track analysis because no analyzer initialized.";
        }
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
void AnalyzerQueue::emitUpdateProgress(TrackPointer track, int progress) {
    if (!m_exitPendingFlag) {
        // First tryAcqire will have always success because sema is initialized with on
        // The following tries will success if the previous signal was processed in the GUI Thread
        // This prevent the AnalysisQueue from filling up the GUI Thread event Queue
        // 100 % is emitted in any case
        if (progress < 1000 - FINALIZE_PROMILLE && progress > 0) {
            // Signals during processing are not required in any case
            if (!m_progressInfo.sema.tryAcquire()) {
               return;
            }
        } else {
            m_progressInfo.sema.acquire();
        }
        m_progressInfo.current_track = track;
        m_progressInfo.track_progress = progress;
        m_progressInfo.queue_size = m_queueSize;
        emit(updateProgress());
    }
}

//slot
void AnalyzerQueue::slotUpdateProgress() {
    if (m_progressInfo.current_track) {
        m_progressInfo.current_track->setAnalyzerProgress(m_progressInfo.track_progress);
        m_progressInfo.current_track.reset();
    }
    emit(trackProgress(m_progressInfo.track_progress / 10));
    if (m_progressInfo.track_progress == 1000) {
        emit(trackFinished(m_progressInfo.queue_size));
    }
    m_progressInfo.sema.release();
}

void AnalyzerQueue::slotAnalyseTrack(TrackPointer pTrack) {
    // This slot is called from the decks and and samplers when the track was loaded.
    enqueueTrack(pTrack);
    resume();
}

// This is called from the GUI and from the AnalyzerQueue thread
void AnalyzerQueue::enqueueTrack(TrackPointer pTrack) {
    VERIFY_OR_DEBUG_ASSERT(pTrack) {
        return;
    }

    QMutexLocker locked(&m_qm);
    // TODO(XXX): Enqueuing with containment checking of each track
    // has an algorithmic complexity of O(n^2)!
    if (!m_queuedTracks.contains(pTrack)) {
        m_queuedTracks.enqueue(pTrack);
        m_queueModifiedFlag = true;
        // Don't wake up paused threads now to avoid race conditions
        // if multiple threads are added in a row. The caller is
        // responsible to finish the enqueuing of tracks with resume().
    }
}
