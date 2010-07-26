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
    void workerReady(QObject* pWorker);
    void workerStarted(QObject* pWorker);
    void workerFinished(QObject* pWorker);

  private:
    QSet<EngineWorker*> m_scheduledWorkers;
    QSet<EngineWorker*> m_activeWorkers;
    QMutex m_mutex;

    QSignalMapper m_readyMapper;
    QSignalMapper m_startedMapper;
    QSignalMapper m_finishedMapper;
};


#endif /* ENGINEWORKERSCHEDULER_H */
