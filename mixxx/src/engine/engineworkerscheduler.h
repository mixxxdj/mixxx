#ifndef ENGINEWORKERSCHEDULER_H
#define ENGINEWORKERSCHEDULER_H

#include <QMutex>
#include <QSet>
#include <QThreadPool>
#include <QWaitCondition>

#include "util/fifo.h"

// The max engine workers that can be expected to run within a callback
// (e.g. the max that we will schedule). Must be a power of 2.
#define MAX_ENGINE_WORKERS 32
// The max number of threads that EngineWorkers will be scheduled on. TODO(XXX)
// this should be dynamically chosen by the user, since it will vary depending
// on the machine resources available.
#define ENGINE_WORKER_THREAD_COUNT 4

class EngineWorker;

class EngineWorkerScheduler : public QThread {
    Q_OBJECT
  public:
    EngineWorkerScheduler(QObject* pParent=NULL);
    virtual ~EngineWorkerScheduler();

    void bindWorker(EngineWorker* pWorker);
    void runWorkers();
    void workerReady(EngineWorker* worker);

  protected:
    void run();

  private:
    FIFO<EngineWorker*> m_scheduleFIFO;
    QThreadPool m_workerThreadPool;
    QWaitCondition m_waitCondition;
    QMutex m_mutex;
    volatile bool m_bQuit;
};

#endif /* ENGINEWORKERSCHEDULER_H */
