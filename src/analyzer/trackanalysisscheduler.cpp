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

//static
TrackAnalysisScheduler::Pointer TrackAnalysisScheduler::nullPointer() {
    return Pointer(nullptr, [](TrackAnalysisScheduler*){});
}

//static
TrackAnalysisScheduler::Pointer TrackAnalysisScheduler::createInstance(
        Library* library,
        int numWorkerThreads,
        const UserSettingsPointer& pConfig,
        AnalyzerMode mode) {
    return Pointer(new TrackAnalysisScheduler(
            library,
            numWorkerThreads,
            pConfig,
            mode),
            deleteTrackAnalysisScheduler);
}

TrackAnalysisScheduler::TrackAnalysisScheduler(
        Library* library,
        int numWorkerThreads,
        const UserSettingsPointer& pConfig,
        AnalyzerMode mode)
        : m_library(library),
          m_currentProgress(kAnalyzerProgressUnknown),
          m_currentCount(0),
          m_finishedCount(0),
          m_dequeuedCount(0),
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
        m_workers.emplace_back(std::make_unique<AnalyzerThread>(
                threadId,
                library->dbConnectionPool(),
                pConfig,
                mode));
        connect(m_workers.back().thread(), SIGNAL(progress(int, AnalyzerThreadState, TrackId, AnalyzerProgress)),
            this, SLOT(onWorkerThreadProgress(int, AnalyzerThreadState, TrackId, AnalyzerProgress)));
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
    if (isFinished()) {
        emit finished();
        return;
    }

    const auto now = Clock::now();
    if (now < (m_lastProgressEmittedAt + kProgressInhibitDuration)) {
        // Inhibit signal
        return;
    }
    m_lastProgressEmittedAt = now;

    if (isFinished()) {
        m_currentProgress = kAnalyzerProgressDone;
    } else {
        AnalyzerProgress workerProgressSum = 0;
        int workerProgressCount = 0;
        for (const auto& worker: m_workers) {
            if (worker.analyzerProgress() >= kAnalyzerProgressNone) {
                workerProgressSum += worker.analyzerProgress();
                ++workerProgressCount;
            }
        }
        // The following algorithm/heuristic shows the user a simple and
        // almost linear progress display when multiple threads are running
        // in parallel. It also covers the expected behavior for the single-
        // threaded case. The observer does not need to know how many threads
        // are actually processing tracks concurrently behind the scenes. We
        // are actually reporting a "fake" progress, but one that fulfills
        // its purpose very well.
        if (workerProgressCount > 0) {
            DEBUG_ASSERT(kAnalyzerProgressNone == 0);
            DEBUG_ASSERT(kAnalyzerProgressDone == 1);
            const int inProgressCount =
                    math_max(1, int(std::ceil(workerProgressSum)));
            const AnalyzerProgress currentProgress =
                    workerProgressSum - std::floor(workerProgressSum);
            // (m_finishedCount + inProgressCount) might exceed m_dequeuedCount
            // in some situations due to race conditions! The minimum of those
            // values is an appropriate choice for reporting progress.
            const int currentCount =
                    math_min(m_finishedCount + inProgressCount, m_dequeuedCount);
            // The combination of the values current count (primary) and current
            // progress (secondary) should never decrease to avoid confusion
            if (m_currentCount < currentCount) {
                m_currentCount = currentCount;
                // Unconditional progress update
                m_currentProgress = currentProgress;
            } else if (m_currentCount == currentCount) {
                // Conditional progress update if current count didn't change
                if (m_currentProgress >= kAnalyzerProgressNone) {
                    // Current progress should not decrease while the count is constant
                    m_currentProgress = math_max(m_currentProgress, currentProgress);
                } else {
                    // Initialize current progress
                    m_currentProgress = currentProgress;
                }
            }
        } else {
            if (m_currentCount < m_finishedCount) {
                m_currentCount = m_finishedCount;
            }
        }
    }
    const int totalCount =
            m_dequeuedCount + m_queuedTrackIds.size();
    DEBUG_ASSERT(m_finishedCount <= m_currentCount);
    DEBUG_ASSERT(m_currentCount <= m_dequeuedCount);
    DEBUG_ASSERT(m_dequeuedCount <= totalCount);
    emit progress(
            m_currentProgress,
            m_currentCount,
            totalCount);
}

void TrackAnalysisScheduler::onWorkerThreadProgress(int threadId, AnalyzerThreadState threadState, TrackId trackId, AnalyzerProgress analyzerProgress) {
    auto& worker = m_workers.at(threadId);
    switch (threadState) {
    case AnalyzerThreadState::Void:
        break;
    case AnalyzerThreadState::Idle:
        worker.receiveThreadIdle();
        submitNextTrack(&worker);
        break;
    case AnalyzerThreadState::Busy:
        worker.receiveAnalyzerProgress(trackId, analyzerProgress);
        emit trackProgress(trackId, analyzerProgress);
        break;
    case AnalyzerThreadState::Done:
        worker.receiveAnalyzerProgress(trackId, analyzerProgress);
        emit trackProgress(trackId, analyzerProgress);
        ++m_finishedCount;
        DEBUG_ASSERT(m_finishedCount <= m_dequeuedCount);
        break;
    case AnalyzerThreadState::Exit:
        worker.receiveThreadExit();
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
    while (!m_queuedTrackIds.empty()) {
        TrackId nextTrackId = m_queuedTrackIds.front();
        DEBUG_ASSERT(nextTrackId.isValid());
        TrackPointer nextTrack = loadTrackById(nextTrackId);
        if (!nextTrack) {
            // Skip unloadable track
            m_queuedTrackIds.pop_front();
            ++m_dequeuedCount;
            ++m_finishedCount;
            continue;
        }
        worker->submitNextTrack(std::move(nextTrack));
        m_queuedTrackIds.pop_front();
        ++m_dequeuedCount;
        worker->wakeThread();
        return true;
    }
    DEBUG_ASSERT(m_finishedCount <= m_dequeuedCount);
    return false;
}

void TrackAnalysisScheduler::stop() {
    kLogger.debug() << "Stopping";
    for (auto& worker: m_workers) {
        worker.stopThread();
    }
}

TrackPointer TrackAnalysisScheduler::loadTrackById(TrackId trackId) {
    VERIFY_OR_DEBUG_ASSERT(trackId.isValid()) {
        return TrackPointer();
    }
    TrackPointer track =
            m_library->trackCollection().getTrackDAO().getTrack(trackId);
    if (!track) {
        kLogger.warning()
                << "Failed to load track with id"
                << trackId;
    }
    return track;
}
