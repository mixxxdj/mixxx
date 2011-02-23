#ifndef ENGINEWORKERSCHEDULER_H
#define ENGINEWORKERSCHEDULER_H

#include <QSignalMapper>
#include <QSet>
#include <QThreadPool>
#include <QMutex>

class EngineWorker;

class EngineWorkerScheduler : public QThreadPool {
    Q_OBJECT
  public:
    EngineWorkerScheduler(QObject* pParent=NULL);
    virtual ~EngineWorkerScheduler();

    void runWorkers();
    void bindWorker(EngineWorker* pWorker);

  private slots:
    void workerReady(EngineWorker* worker);
    void workerStarted(EngineWorker* worker);
    void workerFinished(EngineWorker* worker);

  private:
    QSet<EngineWorker*> m_scheduledWorkers;
    QMutex m_mutex;
};


#endif /* ENGINEWORKERSCHEDULER_H */
