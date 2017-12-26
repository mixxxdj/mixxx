#pragma once

#include <deque>
#include <vector>

#include "analyzer/analyzerthread.h"

#include "util/memory.h"


// forward declaration(s)
class Library;

class AnalyzerQueue : public QObject {
    Q_OBJECT

  public:
    AnalyzerQueue(
            Library* library,
            int numWorkerThreads,
            UserSettingsPointer pConfig,
            AnalyzerMode mode = AnalyzerMode::Default);
    ~AnalyzerQueue() override = default;

    int enqueueTrackId(TrackId trackId);

    // After adding tracks the analysis must be resumed once.
    // This function returns the number of tracks that are
    // currently queued for analysis.
    bool resume();

    void cancel();

  signals:
    void progress(int avgCurrentTrackProgress, int finishedSize, int totalSize);
    void empty(int dequeuedSize);
    void done();

  public slots:
    void slotAnalyzeTrack(TrackPointer track);

  private slots:
    void slotWorkerThreadIdle(int threadId);
    void slotWorkerThreadProgress(int threadId);
    void slotWorkerThreadExit(int threadId);

  private:
    TrackPointer loadTrackById(TrackId trackId);
    void updateProgress(AnalyzerThread& workerThread);
    void emitProgress();

    Library* m_library;

    int m_dequeuedSize;

    int m_finishedSize;

    typedef std::chrono::steady_clock Clock;
    Clock::time_point m_lastProgressEmittedAt;

    std::deque<TrackId> m_queuedTrackIds;

    std::vector<std::unique_ptr<AnalyzerThread>> m_workerThreads;
};
