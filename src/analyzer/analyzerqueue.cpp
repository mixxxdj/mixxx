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

void AnalyzerQueue::emitProgress() {
    const auto now = Clock::now();
    // If all enqueued tracks have been finished the signal is
    // emitted independent of the time when the last signal has
    // been emitted.
    if (hasUnfinishedTracks() &&
            (now < (m_lastProgressEmittedAt + kProgressInhibitDuration))) {
        // Don't emit progress update signal
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
    emit(progress(
            analyzerProgress,
            // (m_finishedCount + inProgressCount) might exceed m_dequeuedCount
            // in some situations due to race conditions! The minimum of those
            // values is an appropriate choice for displaying progress.
            math_min(m_finishedCount + inProgressCount, m_dequeuedCount),
            totalCount()));
}

void AnalyzerQueue::slotWorkerThreadProgress(int threadId, AnalyzerThreadState threadState, TrackId trackId) {
    auto& worker = m_workers.at(threadId);
    switch (threadState) {
    case AnalyzerThreadState::Idle:
        worker.recvThreadIdle();
        resumeIdleWorker(&worker);
        return;
    case AnalyzerThreadState::Busy:
        emit(trackProgress(trackId, worker.recvAnalyzerProgress(trackId)));
        emitProgress();
        return;
    case AnalyzerThreadState::Done:
        ++m_finishedCount;
        DEBUG_ASSERT(m_finishedCount <= m_dequeuedCount);
        emit(trackProgress(trackId, worker.recvAnalyzerProgress(trackId)));
        emitProgress();
        return;
    case AnalyzerThreadState::Void:
        worker.recvThreadExit();
        for (const auto& worker: m_workers) {
            if (worker) {
                // At least one thread is left
                emitProgress();
                return;
            }
        }
        emit(done());
        return;
    }
    DEBUG_ASSERT(!"Unhandled signal from worker thread");
}

void AnalyzerQueue::enqueueTrackId(TrackId trackId) {
    VERIFY_OR_DEBUG_ASSERT(trackId.isValid()) {
        qWarning()
                << "Cannot enqueue track with invalid id"
                << trackId;
        return;
    }
    m_queuedTrackIds.push_back(trackId);
    // Don't wake up the paused thread now to avoid race conditions
    // if multiple threads are added in a row by calling this function
    // multiple times. The caller is responsible to finish the enqueuing
    // of tracks with resume().
}

void AnalyzerQueue::pause() {
    for (auto& worker: m_workers) {
        worker.pauseThread();
    }
}

void AnalyzerQueue::resume() {
    bool resumedIdleWorker = false;
    for (auto& worker: m_workers) {
        if (worker.threadIdle()) {
            if (resumeIdleWorker(&worker)) {
                resumedIdleWorker = true;
            }
        } else {
            worker.resumeThread();
        }
    }
    if (!resumedIdleWorker) {
        emitProgress();
    }
}

bool AnalyzerQueue::resumeIdleWorker(Worker* worker) {
    DEBUG_ASSERT(worker);
    DEBUG_ASSERT(worker->threadIdle());
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
        worker->sendNextTrack(nextTrack);
        m_queuedTrackIds.pop_front();
        ++m_dequeuedCount;
        emitProgress();
        return true;
    }
    emitProgress();
    DEBUG_ASSERT(m_finishedCount <= m_dequeuedCount);
    if (m_finishedCount == m_dequeuedCount) {
        emit(empty(m_finishedCount));
    }
    return false;
}

void AnalyzerQueue::cancel() {
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
        m_impl->cancel();
        // Avoid blocking the event loop on the worker thread's destructors!
        m_impl->deleteLater();
        // Release ownership
        m_impl.release();
        DEBUG_ASSERT(!m_impl);
    }
}
