#pragma once

#include <QQueue>

#include "analyzer/analyzerthread.h"


// forward declaration(s)
class Library;

class AnalyzerQueue : public QObject {
    Q_OBJECT

  public:
    AnalyzerQueue(
            Library* library,
            UserSettingsPointer pConfig,
            AnalyzerMode mode = AnalyzerMode::Default);
    ~AnalyzerQueue() override = default;

    int enqueueTrack(TrackId trackId);

    // After adding tracks the analysis must be resumed.
    // This function returns the number of tracks that
    // are currently queued for analysis.
    void resumeAnalysis();

    void cancelAnalysis();

  signals:
    void analysisProgress(int currentTrackProgress, int dequeuedSize, int enqueuedSize);
    void threadIdle();

  public slots:
    void slotAnalyseTrack(TrackPointer track);

  private slots:
    void slotThreadProgressUpdate();
    void slotThreadIdle();

  private:
    TrackPointer loadTrack(TrackId trackId);
    void emitAnalysisProgress(int currentTrackProgress = kAnalysisProgressUnknown);

    Library* m_library;

    int m_dequeuedSize;

    QQueue<TrackId> m_queuedTrackIds;

    AnalyzerThread m_thread;
};
