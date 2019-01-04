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
          m_finishedTracksCount(0),
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
        emit finished();
        return;
    }

    const auto now = Clock::now();
    if (now < (m_lastProgressEmittedAt + kProgressInhibitDuration)) {
        // Inhibit signal
        return;
    }
    m_lastProgressEmittedAt = now;

    if (allTracksFinished()) {
        m_currentTrackProgress = kAnalyzerProgressDone;
    } else {
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
            // = m_finishedTracksCount + inProgressCount exceeds the upper
            // bound m_dequeuedTracksCount. Using the minimum of both values
            // is an appropriate choice for reporting continuous progress.
            const int currentTrackNumber =
                    math_min(m_finishedTracksCount + inProgressCount, m_dequeuedTracksCount);
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
            if (m_currentTrackNumber < m_finishedTracksCount) {
                m_currentTrackNumber = m_finishedTracksCount;
            }
        }
    }
    const int totalTracksCount =
            m_dequeuedTracksCount + m_queuedTrackIds.size();
    DEBUG_ASSERT(m_finishedTracksCount <= m_currentTrackNumber);
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
    auto& worker = m_workers.at(threadId);
    switch (threadState) {
    case AnalyzerThreadState::Void:
        break;
    case AnalyzerThreadState::Idle:
        worker.onThreadIdle();
        submitNextTrack(&worker);
        break;
    case AnalyzerThreadState::Busy:
        worker.onAnalyzerProgress(trackId, analyzerProgress);
        emit trackProgress(trackId, analyzerProgress);
        break;
    case AnalyzerThreadState::Done:
        worker.onAnalyzerProgress(trackId, analyzerProgress);
        emit trackProgress(trackId, analyzerProgress);
        ++m_finishedTracksCount;
        DEBUG_ASSERT(m_finishedTracksCount <= m_dequeuedTracksCount);
        break;
    case AnalyzerThreadState::Exit:
        worker.onThreadExit();
        DEBUG_ASSERT(!worker);
        break;
    default:
        DEBUG_ASSERT(!"Unhandled signal from worker thread");
    }
    emitProgressOrFinished();
}

void TrackAnalysisScheduler::scheduleTrackById(TrackId trackId) {
    VERIFY_OR_DEBUG_ASSERT(trackId.isValid()) {
        qWarning()
                << "Cannot schedule track with invalid id"
                << trackId;
        return;
    }
    m_queuedTrackIds.push_back(trackId);
    // Don't wake up the suspended thread now to avoid race conditions
    // if multiple threads are added in a row by calling this function
    // multiple times. The caller is responsible to finish the scheduling
    // of multiple tracks with resume().
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
        if (worker.threadIdle()) {
            submitNextTrack(&worker);
        }
        worker.resumeThread();
    }
}

bool TrackAnalysisScheduler::submitNextTrack(Worker* worker) {
    DEBUG_ASSERT(worker);
    DEBUG_ASSERT(worker->threadIdle());
    while (!m_queuedTrackIds.empty()) {
        TrackId nextTrackId = m_queuedTrackIds.front();
        DEBUG_ASSERT(nextTrackId.isValid());
        if (nextTrackId.isValid()) {
            TrackPointer nextTrack =
                    m_library->trackCollection().getTrackDAO().getTrack(nextTrackId);
            if (nextTrack) {
                VERIFY_OR_DEBUG_ASSERT(worker->submitNextTrack(std::move(nextTrack))) {
                    // This will and must never happen! We will only submit the next
                    // track only after the worker has signaled that it is idle. In
                    // this case the lock-free FIFO for passing data between threads
                    // is empty and enqueueing is expected to succeed.
                    kLogger.critical()
                            << "Failed to submit next track ...retrying...";
                    // Retry to avoid skipping this track
                    continue;
                }
                m_queuedTrackIds.pop_front();
                ++m_dequeuedTracksCount;
                worker->wakeThread();
                return true;
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
        ++m_finishedTracksCount;
    }
    DEBUG_ASSERT(m_finishedTracksCount <= m_dequeuedTracksCount);
    return false;
}

void TrackAnalysisScheduler::stop() {
    kLogger.debug() << "Stopping";
    m_queuedTrackIds.clear();
    for (auto& worker: m_workers) {
        worker.stopThread();
    }
    // The worker threads are still running at this point
    // and m_workers must not be modified!
}
