#pragma once

#include <QQueue>

#include "analyzer/analyzerthread.h"


class AnalyzerQueue : public QObject {
    Q_OBJECT

  public:
    AnalyzerQueue(
            mixxx::DbConnectionPoolPtr pDbConnectionPool,
            UserSettingsPointer pConfig,
            AnalyzerMode mode = AnalyzerMode::Default);
    ~AnalyzerQueue() override = default;

    int enqueueTrack(TrackPointer track);

    // After adding tracks the analysis must be resumed.
    // This function returns the number of tracks that
    // are currently queued for analysis.
    void resumeAnalysis();

    void cancelAnalysis();

  signals:
    void trackProgress(int progress);
    void trackFinished(int queueSize);
    void threadIdle();

  public slots:
    void slotAnalyseTrack(TrackPointer track);

  private slots:
    void slotThreadProgressUpdate();
    void slotThreadIdle();

  private:
    QQueue<TrackPointer> m_queuedTracks;

    AnalyzerThread m_thread;
};
