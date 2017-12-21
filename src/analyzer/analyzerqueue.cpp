#include "analyzer/analyzerqueue.h"

#include "util/logger.h"


namespace {

mixxx::Logger kLogger("AnalyzerQueue");

} // anonymous namespace

AnalyzerQueue::AnalyzerQueue(
        mixxx::DbConnectionPoolPtr pDbConnectionPool,
        UserSettingsPointer pConfig,
        AnalyzerMode mode)
        : m_thread(
                std::move(pDbConnectionPool),
                std::move(pConfig),
                mode) {

    connect(&m_thread, SIGNAL(progressUpdate()),
            this, SLOT(slotThreadProgressUpdate()));
    connect(&m_thread, SIGNAL(idle()),
            this, SLOT(slotThreadIdle()));

    m_thread.start(QThread::LowPriority);
}

void AnalyzerQueue::resumeAnalysis() {
    TrackPointer nextTrack;
    if (!m_queuedTracks.empty()) {
        nextTrack = m_queuedTracks.head();
    }
    if (m_thread.wake(nextTrack) && nextTrack) {
        m_queuedTracks.dequeue();
    }
}

void AnalyzerQueue::cancelAnalysis() {
    m_queuedTracks.clear();
    m_thread.stop();
}

void AnalyzerQueue::slotThreadProgressUpdate() {
    const auto readScope = m_thread.readProgressUpdate();
    if (readScope) {
        //kLogger.trace() << "Reading progress info";
        // TODO(XXX): Bulk progress updates of many track objects
        // that have been skipped during analysis can still limit
        // the responsiveness of the UI.
        bool oneOrMoreTracksFinished = false;
        for (const auto trackWithProgress: readScope.tracksWithProgress()) {
            if (trackWithProgress.second != kAnalysisProgressUnknown) {
                trackWithProgress.first->setAnalysisProgress(trackWithProgress.second);
                if (trackWithProgress.second == kAnalysisProgressDone) {
                    oneOrMoreTracksFinished = true;
                }
            }
        }
        DEBUG_ASSERT(oneOrMoreTracksFinished ||
                (readScope.currentTrackProgress() != kAnalysisProgressDone));
        if (readScope.currentTrackProgress() != kAnalysisProgressUnknown) {
            int trackProgressPercent = readScope.currentTrackProgress() / 10;
            emit(trackProgress(trackProgressPercent));
        }
        if (oneOrMoreTracksFinished) {
            emit(trackFinished(m_queuedTracks.size()));
        }
    }
}

void AnalyzerQueue::slotThreadIdle() {
    if (m_queuedTracks.empty()) {
        emit(threadIdle());
    } else {
        m_thread.wake(m_queuedTracks.dequeue());
    }
}

void AnalyzerQueue::slotAnalyseTrack(TrackPointer track) {
    // This slot is called from the decks and and samplers when the track was loaded.
    enqueueTrack(track);
    resumeAnalysis();
}

int AnalyzerQueue::enqueueTrack(TrackPointer track) {
    VERIFY_OR_DEBUG_ASSERT(track) {
        return m_queuedTracks.size();
    }
    kLogger.debug()
            << "Enqueuing track"
            << track->getLocation();
    m_queuedTracks.enqueue(std::move(track));
    // Don't wake up the paused thread now to avoid race conditions
    // if multiple threads are added in a row. The caller is
    // responsible to finish the enqueuing of tracks with resumeThread().
    return m_queuedTracks.size();
}
