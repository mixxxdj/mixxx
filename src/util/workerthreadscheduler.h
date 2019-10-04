#pragma once

#include "util/workerthread.h"
#include "util/spscfifo.h"


// Non-blocking scheduler for worker threads which itself runs
// as a worker thread. The maximum number of worker threads is
// limited.
// TODO: Currently unused, supposed to be used for scheduling
// engine worker threads.
class WorkerThreadScheduler : public WorkerThread {
  public:
    explicit WorkerThreadScheduler(
            size_t maxWorkers,
            const QString& name = QString());
    ~WorkerThreadScheduler() override = default;

    bool scheduleWorker(WorkerThread* worker);

  protected:
    void doRun() override;

    FetchWorkResult tryFetchWorkItems() override;

  private:
    SpscFifo<WorkerThread*> m_scheduledWorkers;

    WorkerThread* m_fetchedWorker;
};
