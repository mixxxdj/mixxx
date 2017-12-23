#include "analyzer/analyzerqueue.h"

#include "library/library.h"
#include "library/trackcollection.h"

#include "util/logger.h"


namespace {

mixxx::Logger kLogger("AnalyzerQueue");

constexpr QThread::Priority kThreadPriority = QThread::LowPriority;

} // anonymous namespace

AnalyzerQueue::AnalyzerQueue(
        Library* library,
        UserSettingsPointer pConfig,
        AnalyzerMode mode)
        : m_library(library),
          m_dequeuedSize(0),
          m_workerThread(
                  library->dbConnectionPool(),
                  std::move(pConfig),
                  mode) {

    connect(&m_workerThread, SIGNAL(progress()),
            this, SLOT(slotWorkerThreadProgress()));
    connect(&m_workerThread, SIGNAL(idle()),
            this, SLOT(slotWorkerThreadIdle()));
    connect(&m_workerThread, SIGNAL(exit()),
            this, SLOT(slotWorkerThreadExit()));

    m_workerThread.start(kThreadPriority);
}

void AnalyzerQueue::emitProgress(int currentTrackProgress) {
    emit(progress(currentTrackProgress, m_dequeuedSize, m_queuedTrackIds.size()));
}

void AnalyzerQueue::slotWorkerThreadProgress() {
    readWorkerThreadProgress();
}

void AnalyzerQueue::slotWorkerThreadIdle() {
    // Consume any pending progress updates first
    readWorkerThreadProgress();
    while (!m_queuedTrackIds.empty()) {
        emit(progress(kAnalysisProgressUnknown, m_dequeuedSize, m_queuedTrackIds.size()));
        TrackId nextTrackId = m_queuedTrackIds.head();
        DEBUG_ASSERT(nextTrackId.isValid());
        TrackPointer nextTrack = loadTrackById(nextTrackId);
        if (!nextTrack) {
            // Skip unloadable track
            m_queuedTrackIds.dequeue();
            ++m_dequeuedSize;
            continue;
        }
        if (!m_workerThread.wake(nextTrack)) {
            // Try again later
            return;
        }
        m_queuedTrackIds.dequeue();
        ++m_dequeuedSize;
        kLogger.debug()
                << "Continuing analysis with next track"
                << nextTrack->getLocation();
        // Exit loop and function to continue with analysis
        return;
    }
    m_workerThread.stop();
}

void AnalyzerQueue::slotWorkerThreadExit() {
    emit(done());
}

void AnalyzerQueue::slotAnalyseTrack(TrackPointer track) {
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
    m_queuedTrackIds.enqueue(trackId);
    // Don't wake up the paused thread now to avoid race conditions
    // if multiple threads are added in a row. The caller is
    // responsible to finish the enqueuing of tracks with resumeThread().
    return m_queuedTrackIds.size();
}

void AnalyzerQueue::resume() {
    if (m_workerThread) {
        m_workerThread.wake();
    } else {
        qWarning()
                << "No worker thread";
    }
}

void AnalyzerQueue::cancel() {
    m_workerThread.stop();
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

bool AnalyzerQueue::readWorkerThreadProgress() {
    const auto readScope = m_workerThread.readProgress();
    if (readScope) {
        for (const auto trackWithProgress: readScope.tracksWithProgress()) {
            if (trackWithProgress.second != kAnalysisProgressUnknown) {
                trackWithProgress.first->setAnalysisProgress(trackWithProgress.second);
            }
        }
        emitProgress(readScope.currentTrackProgress());
        return true;
    } else {
        // Check if all pending updates have been consumed so far
        // for deciding whether it is safe to signal idleness of
        // the queue.
        return readScope.empty();
    }
}
