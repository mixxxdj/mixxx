#pragma once

#include <atomic>
#include <QObject>
#include <QSemaphore>
#include <QThread>

// EngineWorker is an interface for running background processing work when the
// audio callback is not active. While the audio callback is active, an
// EngineWorker can emit its workReady signal, and an EngineWorkerManager will
// schedule it for running after the audio callback has completed.

class EngineWorkerScheduler;

class EngineWorker : public QThread {
    Q_OBJECT
  public:
    EngineWorker();
    virtual ~EngineWorker();

    virtual void run();

    void setScheduler(EngineWorkerScheduler* pScheduler);
    void workReady();
    void wakeIfReady();

  protected:
    QSemaphore m_semaRun;

  private:
    EngineWorkerScheduler* m_pScheduler;
    std::atomic_flag m_notReady;
};
