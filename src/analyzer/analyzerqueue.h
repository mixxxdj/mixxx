#ifndef ANALYZER_ANALYZERQUEUE_H
#define ANALYZER_ANALYZERQUEUE_H

#include <QList>
#include <QThread>
#include <QQueue>
#include <QWaitCondition>
#include <QSemaphore>

#include <vector>

#include "analyzer/analyzer.h"
#include "preferences/usersettings.h"
#include "sources/audiosource.h"
#include "trackinfoobject.h"
#include "util/samplebuffer.h"

class TrackCollection;

class AnalyzerQueue : public QThread {
    Q_OBJECT

  public:
    AnalyzerQueue(TrackCollection* pTrackCollection);
    virtual ~AnalyzerQueue();

    void stop();
    void queueAnalyseTrack(TrackPointer tio);

    static AnalyzerQueue* createDefaultAnalyzerQueue(
            UserSettingsPointer pConfig, TrackCollection* pTrackCollection);
    static AnalyzerQueue* createAnalysisFeatureAnalyzerQueue(
            UserSettingsPointer pConfig, TrackCollection* pTrackCollection);

  public slots:
    void slotAnalyseTrack(TrackPointer tio);
    void slotUpdateProgress();

  signals:
    void trackProgress(int progress);
    void trackDone(TrackPointer track);
    void trackFinished(int size);
    // Signals from AnalyzerQueue Thread:
    void queueEmpty();
    void updateProgress();

  protected:
    void run();

  private:
    struct progress_info {
        TrackPointer current_track;
        int track_progress; // in 0.1 %
        int queue_size;
        QSemaphore sema;
    };

    void addAnalyzer(Analyzer* an);

    QList<Analyzer*> m_aq;

    bool isLoadedTrackWaiting(TrackPointer analysingTrack);
    TrackPointer dequeueNextBlocking();
    bool doAnalysis(TrackPointer tio, Mixxx::AudioSourcePointer pAudioSource);
    void emitUpdateProgress(TrackPointer tio, int progress);
    void emptyCheck();

    bool m_exit;
    QAtomicInt m_aiCheckPriorities;

    SampleBuffer m_sampleBuffer;

    // The processing queue and associated mutex
    QQueue<TrackPointer> m_tioq;
    QMutex m_qm;
    QWaitCondition m_qwait;
    struct progress_info m_progressInfo;
    int m_queue_size;
};

#endif /* ANALYZER_ANALYZERQUEUE_H */
