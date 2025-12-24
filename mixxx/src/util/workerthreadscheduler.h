#pragma once

#include "util/workerthread.h"
#include "util/fifo.h"

/// Non-blocking scheduler for worker threads which itself runs
/// as a worker thread. The maximum number of worker threads is
/// limited.
class WorkerThreadScheduler : public WorkerThread {
    Q_OBJECT
  public:
    explicit WorkerThreadScheduler(
            int maxWorkers,
            const QString& name = QString());
    ~WorkerThreadScheduler() override = default;

    bool scheduleWorker(WorkerThread* worker);

    bool resumeWorkers();

  protected:
    void doRun() override;

    TryFetchWorkItemsResult tryFetchWorkItems() override;

  private:
    FIFO<WorkerThread*> m_scheduledWorkers;

    WorkerThread* m_fetchedWorker;
};
