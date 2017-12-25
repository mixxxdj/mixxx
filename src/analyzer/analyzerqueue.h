#pragma once

#include <array>
#include <deque>

#include "analyzer/analyzerthread.h"

#include "util/memory.h"


// forward declaration(s)
class Library;

class AnalyzerQueue : public QObject {
    Q_OBJECT

  public:
    // TODO: Use multiple worker threads
    static constexpr int kWorkerThreadCount = 1;

    AnalyzerQueue(
            Library* library,
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
    void progress(int currentTrackProgress, int dequeuedSize, int enqueuedSize);
    void empty();
    void done();

  public slots:
    void slotAnalyzeTrack(TrackPointer track);

  private slots:
    void slotWorkerThreadProgress(int threadId);
    void slotWorkerThreadIdle(int threadId);
    void slotWorkerThreadExit(int threadId);

  private:
    TrackPointer loadTrackById(TrackId trackId);
    void readWorkerThreadProgress(int threadId);
    void emitProgress(int threadId, int currentTrackProgress = kAnalyzerProgressUnknown);

    Library* m_library;

    int m_dequeuedSize;

    std::deque<TrackId> m_queuedTrackIds;

    std::array<std::unique_ptr<AnalyzerThread>, kWorkerThreadCount> m_workerThreads;
};
