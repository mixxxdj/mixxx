#include "analyzer/analyzerqueue.h"

#include "library/library.h"
#include "library/trackcollection.h"

#include "util/logger.h"


namespace {

mixxx::Logger kLogger("AnalyzerQueue");

constexpr QThread::Priority kWorkerThreadPriority = QThread::LowPriority;

} // anonymous namespace

AnalyzerQueue::AnalyzerQueue(
        Library* library,
        UserSettingsPointer pConfig,
        AnalyzerMode mode)
        : m_library(library),
          m_dequeuedSize(0) {
    for (int threadId = 0; threadId < kWorkerThreadCount; ++threadId) {
        auto workerThread = std::make_unique<AnalyzerThread>(
                threadId,
                library->dbConnectionPool(),
                std::move(pConfig),
                mode);
        connect(workerThread.get(), SIGNAL(progress(int)),
            this, SLOT(slotWorkerThreadProgress(int)));
        connect(workerThread.get(), SIGNAL(idle(int)),
            this, SLOT(slotWorkerThreadIdle(int)));
        connect(workerThread.get(), SIGNAL(exit(int)),
            this, SLOT(slotWorkerThreadExit(int)));
        m_workerThreads[threadId] = std::move(workerThread);
        m_workerThreads[threadId]->start(kWorkerThreadPriority);
    }
}

void AnalyzerQueue::emitProgress(int /*threadId*/, int currentTrackProgress) {
    // TODO(XXX): Emitting progress of the current track from one of multiple
    // threads doesn't really make sense without distinguishing thh different
    // threads within the signal.
    DEBUG_ASSERT(kWorkerThreadCount == 1);
    emit(progress(currentTrackProgress, m_dequeuedSize, m_queuedTrackIds.size()));
}

void AnalyzerQueue::slotWorkerThreadProgress(int threadId) {
    readWorkerThreadProgress(threadId);
}

void AnalyzerQueue::slotWorkerThreadIdle(int threadId) {
    // Consume any pending progress updates first
    readWorkerThreadProgress(threadId);
    while (!m_queuedTrackIds.empty()) {
        emitProgress(threadId);
        TrackId nextTrackId = m_queuedTrackIds.front();
        DEBUG_ASSERT(nextTrackId.isValid());
        TrackPointer nextTrack = loadTrackById(nextTrackId);
        if (!nextTrack) {
            // Skip unloadable track
            m_queuedTrackIds.pop_front();
            ++m_dequeuedSize;
            continue;
        }
        const auto& workerThread = m_workerThreads[threadId];
        if (!workerThread || !workerThread->wake(nextTrack)) {
            // Try again later
            return;
        }
        m_queuedTrackIds.pop_front();
        ++m_dequeuedSize;
        kLogger.debug()
                << "Continuing analysis with next track"
                << nextTrack->getLocation();
        // Exit loop and function to continue with analysis
        return;
    }
    readWorkerThreadProgress(threadId);
    emit(empty());
}

void AnalyzerQueue::slotWorkerThreadExit(int threadId) {
    auto workerThread = std::move(m_workerThreads[threadId]);
    DEBUG_ASSERT(!m_workerThreads[threadId]);
    if (workerThread) {
        workerThread->deleteLater();
        workerThread.release();
    }
    for (const auto& workerThread: m_workerThreads) {
        if (workerThread) {
            // At least one thread left that has not exited
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
    kLogger.debug()
            << "Enqueuing track with id"
            << trackId;
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

void AnalyzerQueue::readWorkerThreadProgress(int threadId) {
    const auto& workerThread = m_workerThreads[threadId];
    if (workerThread) {
        const auto readScope = workerThread->readProgress();
        if (readScope) {
            for (const auto trackWithProgress: readScope.tracksWithProgress()) {
                if (trackWithProgress.second != kAnalyzerProgressUnknown) {
                    trackWithProgress.first->setAnalyzerProgress(trackWithProgress.second);
                }
            }
            emitProgress(threadId, readScope.currentTrackProgress());
        }
    }
}
