#ifndef ANALYSERQUEUE_H
#define ANALYSERQUEUE_H

#include <QList>
#include <QThread>
#include <QQueue>
#include <QWaitCondition>

#include "configobject.h"
#include "analyser.h"
#include "trackinfoobject.h"

class SoundSourceProxy;

class AnalyserQueue : public QThread {
    Q_OBJECT

  public:
    AnalyserQueue();
    virtual ~AnalyserQueue();
    void stop();

    static AnalyserQueue* createDefaultAnalyserQueue(ConfigObject<ConfigValue> *_config);
    static AnalyserQueue* createPrepareViewAnalyserQueue(ConfigObject<ConfigValue> *_config);
    static AnalyserQueue* createAnalyserQueue(QList<Analyser*> analysers);

  public slots:
    void queueAnalyseTrack(TrackPointer tio);

  signals:
    void trackProgress(TrackPointer pTrack, int progress);
    void trackFinished(TrackPointer pTrack, int queue_size);
    void queueEmpty();
    void updateProgress();

  protected:
    void run();

  private:
    void addAnalyser(Analyser* an);

    QList<Analyser*> m_aq;

    bool isLoadedTrackWaiting();
    TrackPointer dequeueNextBlocking();
    bool doAnalysis(TrackPointer tio, SoundSourceProxy *pSoundSource);
    void emitTrackProgress(TrackPointer pTrack, int progress);
    void emitTrackFinished(TrackPointer pTrack, int queue_size);

    bool m_exit;
    QAtomicInt m_aiCheckPriorities;

    // The processing queue and associated mutex
    QQueue<TrackPointer> m_tioq;
    QMutex m_qm;
    QWaitCondition m_qwait;
    int m_iTrackProgress;
};

#endif
