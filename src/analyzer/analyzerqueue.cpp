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
          m_thread(
                  library->dbConnectionPool(),
                  std::move(pConfig),
                  mode) {

    connect(&m_thread, SIGNAL(progressUpdate()),
            this, SLOT(slotThreadProgressUpdate()));
    connect(&m_thread, SIGNAL(idle()),
            this, SLOT(slotThreadIdle()));

    m_thread.start(kThreadPriority);
}

void AnalyzerQueue::resumeAnalysis() {
    m_thread.wake();
}

void AnalyzerQueue::cancelAnalysis() {
    m_queuedTrackIds.clear();
    m_thread.stop();
}

void AnalyzerQueue::emitAnalysisProgress(int currentTrackProgress) {
    emit(analysisProgress(currentTrackProgress, m_dequeuedSize, m_queuedTrackIds.size()));
}

void AnalyzerQueue::slotThreadProgressUpdate() {
    const auto readScope = m_thread.readProgressUpdate();
    if (readScope) {
        for (const auto trackWithProgress: readScope.tracksWithProgress()) {
            if (trackWithProgress.second != kAnalysisProgressUnknown) {
                trackWithProgress.first->setAnalysisProgress(trackWithProgress.second);
            }
        }
        emitAnalysisProgress(readScope.currentTrackProgress());
    }
}

void AnalyzerQueue::slotThreadIdle() {
    if (m_queuedTrackIds.empty()) {
        m_dequeuedSize = 0;
        emitAnalysisProgress();
        emit(threadIdle());
    } else {
        while (!m_queuedTrackIds.empty()) {
            emit(analysisProgress(kAnalysisProgressUnknown, m_dequeuedSize, m_queuedTrackIds.size()));
            TrackId nextTrackId = m_queuedTrackIds.head();
            DEBUG_ASSERT(nextTrackId.isValid());
            TrackPointer nextTrack = loadTrack(nextTrackId);
            if (!nextTrack) {
                // Skip unloadable track
                m_queuedTrackIds.dequeue();
                ++m_dequeuedSize;
                continue;
            }
            if (!m_thread.wake(nextTrack)) {
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
        // Wake the thread up one last time and wait for confirmation,
        // i.e. another idle signal while the queue stays empty. This
        // is necessary to avoid a race condition when analyzing the
        // last track from the queue.
        m_thread.wake();
    }
}

void AnalyzerQueue::slotAnalyseTrack(TrackPointer track) {
    // This slot is called from the decks and and samplers when the track was loaded.
    VERIFY_OR_DEBUG_ASSERT(track) {
        return;
    }
    enqueueTrack(track->getId());
    resumeAnalysis();
}

int AnalyzerQueue::enqueueTrack(TrackId trackId) {
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

TrackPointer AnalyzerQueue::loadTrack(TrackId trackId) {
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
