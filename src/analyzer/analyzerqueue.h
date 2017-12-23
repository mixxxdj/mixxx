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

    int enqueueTrackId(TrackId trackId);

    // After adding tracks the analysis must be resumed once.
    // This function returns the number of tracks that are
    // currently queued for analysis.
    void resume();

    void cancel();

  signals:
    void progress(int currentTrackProgress, int dequeuedSize, int enqueuedSize);
    void done();

  public slots:
    void slotAnalyseTrack(TrackPointer track);

  private slots:
    void slotWorkerThreadProgress();
    void slotWorkerThreadIdle();
    void slotWorkerThreadExit();

  private:
    TrackPointer loadTrackById(TrackId trackId);
    bool readWorkerThreadProgress();
    void emitProgress(int currentTrackProgress = kAnalysisProgressUnknown);

    Library* m_library;

    int m_dequeuedSize;

    QQueue<TrackId> m_queuedTrackIds;

    AnalyzerThread m_workerThread;
};
