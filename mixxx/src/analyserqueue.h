#ifndef ANALYSERQUEUE_H
#define ANALYSERQUEUE_H

#include <QList>
#include <QThread>
#include <QQueue>
#include <QWaitCondition>
#include <QSemaphore>

#include "configobject.h"
#include "analyser.h"
#include "trackinfoobject.h"

class SoundSourceProxy;
class TrackCollection;

class AnalyserQueue : public QThread {
    Q_OBJECT

  public:
    AnalyserQueue(TrackCollection* pTrackCollection);
    virtual ~AnalyserQueue();
    void stop();
    void queueAnalyseTrack(TrackPointer tio);

    static AnalyserQueue* createDefaultAnalyserQueue(
            ConfigObject<ConfigValue>* _config, TrackCollection* pTrackCollection);
    static AnalyserQueue* createPrepareViewAnalyserQueue(
            ConfigObject<ConfigValue>* _config, TrackCollection* pTrackCollection);

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

    bool isLoadedTrackWaiting(TrackPointer tio);
    TrackPointer dequeueNextBlocking();
    bool doAnalysis(TrackPointer tio, SoundSourceProxy* pSoundSource);
    void emitUpdateProgress(TrackPointer tio, int progress);

    bool m_exit;
    QAtomicInt m_aiCheckPriorities;

    // The processing queue and associated mutex
    QQueue<TrackPointer> m_tioq;
    QMutex m_qm;
    QWaitCondition m_qwait;
    struct progress_info m_progressInfo;
    int m_queue_size;
};

#endif
