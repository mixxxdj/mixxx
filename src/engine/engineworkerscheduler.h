#pragma once

#include <QMutex>
#include <QThread>
#include <QWaitCondition>

class EngineWorker;

class EngineWorkerScheduler : public QThread {
    Q_OBJECT
  public:
    EngineWorkerScheduler(QObject* pParent = nullptr);
    ~EngineWorkerScheduler() override;

    void addWorker(EngineWorker* pWorker);
    void runWorkers();
    void workerReady();

  protected:
    void run() override;

  private:
    // Indicates whether workerReady has been called since the last time
    // runWorkers was run. This should only be touched from the engine callback.
    bool m_bWakeScheduler;

    // containing pointers are non-owning
    std::vector<EngineWorker*> m_workers;

    QWaitCondition m_waitCondition;
    QMutex m_mutex;
    std::atomic<bool> m_bQuit;
};
