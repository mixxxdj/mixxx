#include "analyzer/analyzerqueue.h"

#include "library/library.h"
#include "library/trackcollection.h"

#include "util/logger.h"


namespace {

mixxx::Logger kLogger("AnalyzerQueue");

constexpr QThread::Priority kWorkerThreadPriority = QThread::LowPriority;

// Maximum frequency of progress updates
constexpr std::chrono::milliseconds kProgressInhibitDuration(100);

} // anonymous namespace

AnalyzerQueue::AnalyzerQueue(
        Library* library,
        int numWorkerThreads,
        const UserSettingsPointer& pConfig,
        AnalyzerMode mode)
        : m_library(library),
          m_dequeuedCount(0),
          m_finishedCount(0),
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
        connect(m_workers.back().thread(), SIGNAL(progress(int, AnalyzerThreadState, TrackId)),
            this, SLOT(slotWorkerThreadProgress(int, AnalyzerThreadState, TrackId)));
    }
    // 2nd pass: Start worker threads
    for (const auto& worker: m_workers) {
        worker.thread()->start(kWorkerThreadPriority);
    }
}

AnalyzerQueue::~AnalyzerQueue() {
    kLogger.debug() << "Destroying";
}

void AnalyzerQueue::emitProgressOrFinished() {
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

    AnalyzerProgress analyzerProgressSum = 0;
    int analyzerProgressCount = 0;
    for (const auto& worker: m_workers) {
        if (worker.analyzerProgress() >= kAnalyzerProgressNone) {
            analyzerProgressSum += worker.analyzerProgress();
            ++analyzerProgressCount;
        }
    }
    // The following algorithm/heuristic shows the user a simple and
    // almost linear progress display when multiple threads are running
    // in parallel. It also covers the expected behavior for the single-
    // threaded case.
    int inProgressCount;
    AnalyzerProgress analyzerProgress;
    if (analyzerProgressCount > 0) {
        DEBUG_ASSERT(kAnalyzerProgressNone == 0);
        DEBUG_ASSERT(kAnalyzerProgressDone == 1);
        inProgressCount = math_max(1, int(std::ceil(analyzerProgressSum)));
        analyzerProgress = analyzerProgressSum - std::floor(analyzerProgressSum);
    } else {
        inProgressCount = 0;
        analyzerProgress = kAnalyzerProgressUnknown;
    }
    // (m_finishedCount + inProgressCount) might exceed m_dequeuedCount
    // in some situations due to race conditions! The minimum of those
    // values is an appropriate choice for displaying progress.
    const int currentCount =
            math_min(m_finishedCount + inProgressCount, m_dequeuedCount);
    const int totalCount =
            m_dequeuedCount + m_queuedTrackIds.size();
    DEBUG_ASSERT(currentCount <= totalCount);
    emit progress(
            analyzerProgress,
            currentCount,
            totalCount);
}

void AnalyzerQueue::slotWorkerThreadProgress(int threadId, AnalyzerThreadState threadState, TrackId trackId) {
    auto& worker = m_workers.at(threadId);
    switch (threadState) {
    case AnalyzerThreadState::Void:
        break;
    case AnalyzerThreadState::Idle:
        worker.receiveThreadIdle();
        submitNextTrack(&worker);
        break;
    case AnalyzerThreadState::Busy:
        emit trackProgress(trackId, worker.receiveAnalyzerProgress(trackId));
        break;
    case AnalyzerThreadState::Done:
        emit trackProgress(trackId, worker.receiveAnalyzerProgress(trackId));
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

void AnalyzerQueue::enqueueTrackId(TrackId trackId) {
    VERIFY_OR_DEBUG_ASSERT(trackId.isValid()) {
        qWarning()
                << "Cannot enqueue track with invalid id"
                << trackId;
        return;
    }
    m_queuedTrackIds.push_back(trackId);
    // Don't wake up the suspended thread now to avoid race conditions
    // if multiple threads are added in a row by calling this function
    // multiple times. The caller is responsible to finish the enqueuing
    // of tracks with resume().
}

void AnalyzerQueue::suspend() {
    kLogger.debug() << "Suspending";
    for (auto& worker: m_workers) {
        worker.suspendThread();
    }
}

void AnalyzerQueue::resume() {
    kLogger.debug() << "Resuming";
    for (auto& worker: m_workers) {
        if (worker.threadIdle()) {
            submitNextTrack(&worker);
        }
        worker.resumeThread();
    }
}

bool AnalyzerQueue::submitNextTrack(Worker* worker) {
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
        worker->writeNextTrack(std::move(nextTrack));
        m_queuedTrackIds.pop_front();
        ++m_dequeuedCount;
        worker->wakeThread();
        return true;
    }
    DEBUG_ASSERT(m_finishedCount <= m_dequeuedCount);
    return false;
}

void AnalyzerQueue::stop() {
    kLogger.debug() << "Stopping";
    for (auto& worker: m_workers) {
        worker.stopThread();
    }
}

TrackPointer AnalyzerQueue::loadTrackById(TrackId trackId) {
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

void AnalyzerQueuePointer::reset() {
    if (m_impl) {
        // Trigger stop
        m_impl->stop();
        // Release ownership and let Qt delete the queue later
        m_impl.release()->deleteLater();
        DEBUG_ASSERT(!m_impl);
    }
}
