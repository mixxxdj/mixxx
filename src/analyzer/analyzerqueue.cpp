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
        UserSettingsPointer pConfig,
        AnalyzerMode mode)
        : m_library(library),
          m_dequeuedSize(0),
          m_finishedSize(0),
          // The first signal should always be emitted
          m_lastProgressEmittedAt(Clock::now() - kProgressInhibitDuration) {
    VERIFY_OR_DEBUG_ASSERT(numWorkerThreads > 0) {
            kLogger.warning()
                    << "Invalid number of worker threads:"
                    << numWorkerThreads;
    } else {
        kLogger.info()
                << "Starting"
                << numWorkerThreads
                << "worker threads";
    }
    // 1st pass: Create worker threads
    m_workerThreads.reserve(numWorkerThreads);
    for (int threadId = 0; threadId < numWorkerThreads; ++threadId) {
        m_workerThreads.push_back(std::make_unique<AnalyzerThread>(
                threadId,
                library->dbConnectionPool(),
                std::move(pConfig),
                mode));
        connect(m_workerThreads.back().get(), SIGNAL(idle(int)),
            this, SLOT(slotWorkerThreadIdle(int)));
        connect(m_workerThreads.back().get(), SIGNAL(progress(int)),
            this, SLOT(slotWorkerThreadProgress(int)));
        connect(m_workerThreads.back().get(), SIGNAL(exit(int)),
            this, SLOT(slotWorkerThreadExit(int)));
    }
    // 2nd pass: Start worker threads
    for (const auto& workerThread: m_workerThreads) {
        workerThread->start(kWorkerThreadPriority);
    }
}

void AnalyzerQueue::emitProgress() {
    const auto now = Clock::now();
    if (now < (m_lastProgressEmittedAt + kProgressInhibitDuration)) {
        // Don't emit progress update signal
        return;
    }
    m_lastProgressEmittedAt = now;

    int partialTrackProgressAmount = 0;
    int partialTrackProgressCount = 0;
    for (const auto& workerThread: m_workerThreads) {
        if (workerThread && workerThread->hasCurrentTrackProgress()) {
            int currentTrackProgress = workerThread->getCurrentTrackProgress();
            // Sum only the amount from worker threads with partial progress
            if (currentTrackProgress < kAnalyzerProgressDone) {
                partialTrackProgressAmount += currentTrackProgress - kAnalyzerProgressNone;
                ++partialTrackProgressCount;
            }
        }
    }
    DEBUG_ASSERT((m_finishedSize + partialTrackProgressCount) <= m_dequeuedSize);
    int currentProgress;
    int finishedSize;
    if (partialTrackProgressCount > 0) {
        currentProgress =
                kAnalyzerProgressNone +
                (partialTrackProgressAmount % (kAnalyzerProgressDone - kAnalyzerProgressNone));
        finishedSize =
                m_finishedSize +
                (partialTrackProgressAmount / (kAnalyzerProgressDone - kAnalyzerProgressNone));
        DEBUG_ASSERT(finishedSize >= 0);
        DEBUG_ASSERT(finishedSize <= m_dequeuedSize);
    } else {
        currentProgress = kAnalyzerProgressUnknown;
        finishedSize = m_finishedSize;
    }
    emit(progress(
            currentProgress,
            finishedSize,
            m_dequeuedSize + m_queuedTrackIds.size()));
}

void AnalyzerQueue::slotWorkerThreadProgress(int threadId) {
    const auto& workerThread = m_workerThreads.at(threadId);
    VERIFY_OR_DEBUG_ASSERT(workerThread) {
        return;
    }
    updateProgress(*workerThread);
    emitProgress();
}

void AnalyzerQueue::slotWorkerThreadIdle(int threadId) {
    //kLogger.debug() << "slotWorkerThreadIdle" << threadId;
    const auto& workerThread = m_workerThreads.at(threadId);
    VERIFY_OR_DEBUG_ASSERT(workerThread) {
        return;
    }
    updateProgress(*workerThread);
    workerThread->setCurrentTrackProgress(kAnalyzerProgressUnknown);
    while (!m_queuedTrackIds.empty()) {
        TrackId nextTrackId = m_queuedTrackIds.front();
        DEBUG_ASSERT(nextTrackId.isValid());
        TrackPointer nextTrack = loadTrackById(nextTrackId);
        if (!nextTrack) {
            // Skip unloadable track
            m_queuedTrackIds.pop_front();
            ++m_dequeuedSize;
            ++m_finishedSize;
            continue;
        }
        if (!workerThread->wake(nextTrack)) {
            // Try again later
            emitProgress();
            return;
        }
        m_queuedTrackIds.pop_front();
        ++m_dequeuedSize;
        workerThread->setCurrentTrackProgress(kAnalyzerProgressNone);
        // Exit loop and function to continue with analysis
        emitProgress();
        return;
    }
    emitProgress();
    DEBUG_ASSERT(m_finishedSize <= m_dequeuedSize);
    if (m_finishedSize == m_dequeuedSize) {
        emit(empty(m_dequeuedSize));
    }
}

void AnalyzerQueue::slotWorkerThreadExit(int threadId) {
    auto& workerThread = m_workerThreads.at(threadId);
    VERIFY_OR_DEBUG_ASSERT(workerThread) {
        return;
    }
    workerThread->deleteLater();
    workerThread.release();
    DEBUG_ASSERT(!workerThread);
    for (const auto& workerThread: m_workerThreads) {
        if (workerThread) {
            // At least one thread left
            return;
        }
    }
    emit(done());
}

void AnalyzerQueue::slotAnalyzeTrack(TrackPointer track) {
    // This slot is called from the decks and and samplers when the track was loaded.
    VERIFY_OR_DEBUG_ASSERT(track) {
        return;
    }
    enqueueTrackId(track->getId());
    resume();
}

int AnalyzerQueue::enqueueTrackId(TrackId trackId) {
    VERIFY_OR_DEBUG_ASSERT(trackId.isValid()) {
        qWarning()
                << "Cannot enqueue track with invalid id"
                << trackId;
        return m_queuedTrackIds.size();
    }
    m_queuedTrackIds.push_back(trackId);
    // Don't wake up the paused thread now to avoid race conditions
    // if multiple threads are added in a row. The caller is
    // responsible to finish the enqueuing of tracks with resumeThread().
    return m_queuedTrackIds.size();
}

bool AnalyzerQueue::resume() {
    bool resumed = false;
    for (const auto& workerThread: m_workerThreads) {
        if (workerThread && workerThread->wake()) {
            resumed = true;
        }
    }
    return resumed;
}

void AnalyzerQueue::cancel() {
    for (const auto& workerThread: m_workerThreads) {
        if (workerThread) {
            workerThread->stop();
        }
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

void AnalyzerQueue::updateProgress(AnalyzerThread& workerThread) {
    const auto readScope = workerThread.readProgress();
    if (readScope) {
        for (const auto trackWithProgress: readScope.tracksWithProgress()) {
            trackWithProgress.first->setAnalyzerProgress(trackWithProgress.second);
        }
        m_finishedSize += readScope.finishedCount();
        if (readScope.currentTrack()) {
            workerThread.setCurrentTrackProgress(readScope.currentTrackProgress());
        } else {
            workerThread.setCurrentTrackProgress(kAnalyzerProgressUnknown);
        }
    }
}
