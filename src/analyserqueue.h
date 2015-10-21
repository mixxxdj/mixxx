#ifndef ANALYSERQUEUE_H
#define ANALYSERQUEUE_H

#include "configobject.h"
#include "analyser.h"
#include "trackinfoobject.h"
#include "sources/audiosource.h"
#include "samplebuffer.h"

#include <QList>
#include <QThread>
#include <QQueue>
#include <QWaitCondition>
#include <QSemaphore>

#include <vector>

class TrackCollection;

class AnalyserQueue : public QThread {
    Q_OBJECT

  public:
    AnalyserQueue(TrackCollection* pTrackCollection);
    virtual ~AnalyserQueue();
    void stop();
    void queueAnalyseTrack(TrackPointer tio);

    static AnalyserQueue* createDefaultAnalyserQueue(
            ConfigObject<ConfigValue>* pConfig, TrackCollection* pTrackCollection);
    static AnalyserQueue* createAnalysisFeatureAnalyserQueue(
            ConfigObject<ConfigValue>* pConfig, TrackCollection* pTrackCollection);

  public slots:
    void slotAnalyseTrack(TrackPointer tio);
    void slotUpdateProgress();

  signals:
    void trackProgress(int progress);
    void trackDone(TrackPointer track);
    void trackFinished(int size);
    // Signals from AnalyserQueue Thread:
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

    void addAnalyser(Analyser* an);

    QList<Analyser*> m_aq;

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

#endif
