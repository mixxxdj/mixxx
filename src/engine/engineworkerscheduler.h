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
    std::atomic<bool> m_bWakeScheduler;

    QWaitCondition m_waitCondition;

    // mutex protects m_workers and m_bQuit
    QMutex m_mutex;
    // containing pointers are non-owning
    std::vector<EngineWorker*> m_workers;
    std::atomic<bool> m_bQuit;
};
