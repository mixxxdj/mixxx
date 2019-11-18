#include "analyzer/trackanalysisscheduler.h"

#include "library/library.h"
#include "library/trackcollection.h"

#include "util/logger.h"


namespace {

mixxx::Logger kLogger("TrackAnalysisScheduler");

constexpr QThread::Priority kWorkerThreadPriority = QThread::LowPriority;

// Maximum frequency of progress updates
constexpr std::chrono::milliseconds kProgressInhibitDuration(100);

void deleteTrackAnalysisScheduler(TrackAnalysisScheduler* plainPtr) {
    if (plainPtr) {
        // Trigger stop
        plainPtr->stop();
        // Release ownership and let Qt delete the queue later
        plainPtr->deleteLater();
    }
}

} // anonymous namespace

TrackAnalysisScheduler::NullPointer::NullPointer()
    : Pointer(nullptr, [](TrackAnalysisScheduler*){}) {
}

//static
TrackAnalysisScheduler::Pointer TrackAnalysisScheduler::createInstance(
        Library* library,
        int numWorkerThreads,
        const UserSettingsPointer& pConfig,
        AnalyzerModeFlags modeFlags) {
    return Pointer(new TrackAnalysisScheduler(
            library,
            numWorkerThreads,
            pConfig,
            modeFlags),
            deleteTrackAnalysisScheduler);
}

TrackAnalysisScheduler::TrackAnalysisScheduler(
        Library* library,
        int numWorkerThreads,
        const UserSettingsPointer& pConfig,
        AnalyzerModeFlags modeFlags)
        : m_library(library),
          m_currentTrackProgress(kAnalyzerProgressUnknown),
          m_currentTrackNumber(0),
          m_dequeuedTracksCount(0),
          // The first signal should always be emitted
          m_lastProgressEmittedAt(Clock::now() - kProgressInhibitDuration) {
    VERIFY_OR_DEBUG_ASSERT(numWorkerThreads > 0) {
            kLogger.warning()
                    << "Invalid number of worker threads:"
                    << numWorkerThreads;
    } else {
        kLogger.debug()
                << "Starting"
                << numWorkerThreads
                << "worker threads";
    }
    // 1st pass: Create worker threads
    m_workers.reserve(numWorkerThreads);
    for (int threadId = 0; threadId < numWorkerThreads; ++threadId) {
        m_workers.emplace_back(AnalyzerThread::createInstance(
                threadId,
                library->dbConnectionPool(),
                pConfig,
                modeFlags));
        connect(m_workers.back().thread(), &AnalyzerThread::progress,
            this, &TrackAnalysisScheduler::onWorkerThreadProgress);
    }
    // 2nd pass: Start worker threads in a suspended state
    for (const auto& worker: m_workers) {
        worker.thread()->suspend();
        worker.thread()->start(kWorkerThreadPriority);
    }
}

TrackAnalysisScheduler::~TrackAnalysisScheduler() {
    kLogger.debug() << "Destroying";
}

void TrackAnalysisScheduler::emitProgressOrFinished() {
    // The finished() signal is emitted regardless of when the last
    // signal has been emitted
    if (allTracksFinished()) {
        m_currentTrackProgress = kAnalyzerProgressUnknown;
        m_currentTrackNumber = 0;
        m_dequeuedTracksCount = 0;
        emit finished();
        return;
    }

    const auto now = Clock::now();
    if (now < (m_lastProgressEmittedAt + kProgressInhibitDuration)) {
        // Inhibit signal
        return;
    }
    m_lastProgressEmittedAt = now;

    DEBUG_ASSERT(m_pendingTrackIds.size() <=
            static_cast<size_t>(m_dequeuedTracksCount));
    const int finishedTracksCount =
            m_dequeuedTracksCount - m_pendingTrackIds.size();

    AnalyzerProgress workerProgressSum = 0;
    int workerProgressCount = 0;
    for (const auto& worker: m_workers) {
        const AnalyzerProgress workerProgress = worker.analyzerProgress();
        if (workerProgress >= kAnalyzerProgressNone) {
            workerProgressSum += workerProgress;
            ++workerProgressCount;
        }
    }
    // The following algorithm/heuristic is used for calculating the
    // amortized analysis progress (current track number + current
    // track progress) across all worker threads. It results in a
    // simple and almost linear progress display when multiple threads
    // are running in parallel. It also covers the expected behavior
    // for the single-threaded case. The receiver of progress updates
    // should not need to know how many threads are actually processing
    // tracks concurrently behind the scenes.
    if (workerProgressCount > 0) {
        DEBUG_ASSERT(kAnalyzerProgressNone == 0);
        DEBUG_ASSERT(kAnalyzerProgressDone == 1);
        const int inProgressCount =
                math_max(1, int(std::ceil(workerProgressSum)));
        const AnalyzerProgress currentTrackProgress =
                workerProgressSum - std::floor(workerProgressSum);
        // The calculation of inProgressCount is only an approximation.
        // In some situations the calculated virtual current track number
        // = finishedTracksCount + inProgressCount exceeds the upper
        // bound m_dequeuedTracksCount. Using the minimum of both values
        // is an appropriate choice for reporting continuous progress.
        const int currentTrackNumber =
                math_min(finishedTracksCount + inProgressCount, m_dequeuedTracksCount);
        // The combination of the values current count (primary) and current
        // progress (secondary) should never decrease to avoid confusion
        if (m_currentTrackNumber < currentTrackNumber) {
            m_currentTrackNumber = currentTrackNumber;
            // Unconditional progress update
            m_currentTrackProgress = currentTrackProgress;
        } else if (m_currentTrackNumber == currentTrackNumber) {
            // Conditional progress update if current count didn't change
            if (m_currentTrackProgress >= kAnalyzerProgressNone) {
                // Current progress should not decrease while the count is constant
                m_currentTrackProgress = math_max(m_currentTrackProgress, currentTrackProgress);
            } else {
                // Initialize current progress
                m_currentTrackProgress = currentTrackProgress;
            }
        }
    } else {
        if (m_currentTrackNumber < finishedTracksCount) {
            m_currentTrackNumber = finishedTracksCount;
        }
    }
    const int totalTracksCount =
            m_dequeuedTracksCount + m_queuedTrackIds.size();
    DEBUG_ASSERT(m_currentTrackNumber <= m_dequeuedTracksCount);
    DEBUG_ASSERT(m_dequeuedTracksCount <= totalTracksCount);
    emit progress(
            m_currentTrackProgress,
            m_currentTrackNumber,
            totalTracksCount);
}

void TrackAnalysisScheduler::onWorkerThreadProgress(
        int threadId,
        AnalyzerThreadState threadState,
        TrackId trackId,
        AnalyzerProgress analyzerProgress) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "onWorkerThreadProgress"
                << threadId
                << int(threadState)
                << trackId
                << analyzerProgress;
    }
    auto& worker = m_workers.at(threadId);
    switch (threadState) {
    case AnalyzerThreadState::Void:
        DEBUG_ASSERT(!trackId.isValid());
        DEBUG_ASSERT(analyzerProgress == kAnalyzerProgressUnknown);
        break;
    case AnalyzerThreadState::Idle:
        DEBUG_ASSERT(!trackId.isValid());
        DEBUG_ASSERT(analyzerProgress == kAnalyzerProgressUnknown);
        worker.onAnalyzerProgress(analyzerProgress);
        submitNextTrack(&worker);
        break;
    case AnalyzerThreadState::Busy:
        DEBUG_ASSERT(trackId.isValid());
        // Ignore delayed signals for tracks that are no longer pending
        if (m_pendingTrackIds.find(trackId) != m_pendingTrackIds.end()) {
            DEBUG_ASSERT(analyzerProgress != kAnalyzerProgressUnknown);
            DEBUG_ASSERT(analyzerProgress < kAnalyzerProgressDone);
            worker.onAnalyzerProgress(analyzerProgress);
            emit trackProgress(trackId, analyzerProgress);
        }
        break;
    case AnalyzerThreadState::Done:
        DEBUG_ASSERT(trackId.isValid());
        // Ignore delayed signals for tracks that are no longer pending
        if (m_pendingTrackIds.find(trackId) != m_pendingTrackIds.end()) {
            DEBUG_ASSERT((analyzerProgress == kAnalyzerProgressDone) // success
                    || (analyzerProgress == kAnalyzerProgressUnknown)); // failure
            m_pendingTrackIds.erase(trackId);
            worker.onAnalyzerProgress(analyzerProgress);
            emit trackProgress(trackId, analyzerProgress);
        }
        break;
    case AnalyzerThreadState::Exit:
        DEBUG_ASSERT(!trackId.isValid());
        DEBUG_ASSERT(analyzerProgress == kAnalyzerProgressUnknown);
        worker.onThreadExit();
        DEBUG_ASSERT(!worker);
        break;
    default:
        DEBUG_ASSERT(!"Unhandled signal from worker thread");
    }
    emitProgressOrFinished();
}

bool TrackAnalysisScheduler::scheduleTrackById(TrackId trackId) {
    VERIFY_OR_DEBUG_ASSERT(trackId.isValid()) {
        qWarning()
                << "Cannot schedule track with invalid id"
                << trackId;
        return false;
    }
    m_queuedTrackIds.push_back(trackId);
    // Don't wake up the suspended thread now to avoid race conditions
    // if multiple threads are added in a row by calling this function
    // multiple times. The caller is responsible to finish the scheduling
    // of multiple tracks with resume().
    return true;
}

int TrackAnalysisScheduler::scheduleTracksById(const QList<TrackId>& trackIds) {
    int scheduledCount = 0;
    for (auto trackId: trackIds) {
        if (scheduleTrackById(std::move(trackId))) {
            ++scheduledCount;
        }
    }
    return scheduledCount;
}

void TrackAnalysisScheduler::suspend() {
    kLogger.debug() << "Suspending";
    for (auto& worker: m_workers) {
        worker.suspendThread();
    }
}

void TrackAnalysisScheduler::resume() {
    kLogger.debug() << "Resuming";
    for (auto& worker: m_workers) {
        worker.resumeThread();
    }
}

bool TrackAnalysisScheduler::submitNextTrack(Worker* worker) {
    DEBUG_ASSERT(worker);
    while (!m_queuedTrackIds.empty()) {
        TrackId nextTrackId = m_queuedTrackIds.front();
        DEBUG_ASSERT(nextTrackId.isValid());
        if (nextTrackId.isValid()) {
            TrackPointer nextTrack =
                    m_library->trackCollection().getTrackDAO().getTrack(nextTrackId);
            if (nextTrack) {
                if (m_pendingTrackIds.insert(nextTrackId).second) {
                    if (worker->submitNextTrack(std::move(nextTrack))) {
                        m_queuedTrackIds.pop_front();
                        ++m_dequeuedTracksCount;
                        return true;
                    } else {
                        // The worker may already have been assigned new tasks
                        // in the mean time, nothing to worry about.
                        m_pendingTrackIds.erase(nextTrackId);
                        kLogger.debug()
                                << "Failed to submit next track - worker thread"
                                << worker->thread()->id()
                                << "is busy";
                        // Early exit to avoid popping the next track from the queue (see below)!
                        return false;
                    }
                } else {
                    // This track is currently analyzed by one of the workers
                    kLogger.debug()
                            << "Skipping duplicate track id"
                            << nextTrackId;
                }
            } else {
                kLogger.warning()
                        << "Failed to load track by id"
                        << nextTrackId;
            }
        } else {
            kLogger.warning()
                    << "Invalid track id"
                    << nextTrackId;
        }
        // Skip this track
        m_queuedTrackIds.pop_front();
        ++m_dequeuedTracksCount;
    }
    return false;
}

void TrackAnalysisScheduler::stop() {
    kLogger.debug() << "Stopping";
    for (auto& worker: m_workers) {
        worker.stopThread();
    }
    // The worker threads are still running at this point
    // and m_workers must not be modified!
    m_queuedTrackIds.clear();
    m_pendingTrackIds.clear();
    DEBUG_ASSERT((allTracksFinished()));
}

QList<TrackId> TrackAnalysisScheduler::stopAndCollectScheduledTrackIds() {
    QList<TrackId> scheduledTrackIds;
    scheduledTrackIds.reserve(m_queuedTrackIds.size() + m_pendingTrackIds.size());
    for (auto queuedTrackId: m_queuedTrackIds) {
        scheduledTrackIds.append(std::move(queuedTrackId));
    }
    for (auto pendingTrackId: m_pendingTrackIds) {
        scheduledTrackIds.append(std::move(pendingTrackId));
    }
    // Stopping the scheduler will clear all queued and pending tracks,
    // so we need to do this after we have collected all scheduled tracks!
    stop();
    return scheduledTrackIds;
}
