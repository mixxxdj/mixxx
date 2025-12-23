#include "util/workerthreadscheduler.h"

#include "moc_workerthreadscheduler.cpp"
#include "util/workerthread.h"


WorkerThreadScheduler::WorkerThreadScheduler(
        int maxWorkers,
        const QString& name)
        : WorkerThread(name.isEmpty() ? QString("WorkerThreadScheduler") : name),
          m_scheduledWorkers(maxWorkers),
          m_fetchedWorker(nullptr) {
}

bool WorkerThreadScheduler::scheduleWorker(WorkerThread* worker) {
    DEBUG_ASSERT(worker);
    const auto written = m_scheduledWorkers.write(&worker, 1) == 1;
    DEBUG_ASSERT((written == 0) || (written == 1));
    return written == 1;
}

bool WorkerThreadScheduler::resumeWorkers() {
    // Resume the scheduler thread if workers have been scheduled
    // in the meantime
    if (m_scheduledWorkers.readAvailable() > 0) {
        resume();
        return true;
    } else {
        return false;
    }
}

WorkerThread::TryFetchWorkItemsResult WorkerThreadScheduler::tryFetchWorkItems() {
    DEBUG_ASSERT(!m_fetchedWorker);
    WorkerThread* worker;
    if (m_scheduledWorkers.read(&worker, 1) == 1) {
        DEBUG_ASSERT(worker);
        m_fetchedWorker = worker;
        return TryFetchWorkItemsResult::Ready;
    } else {
        // Fall asleep after all scheduled workers have have
        // been resumed.
        return TryFetchWorkItemsResult::Idle;
    }
}

void WorkerThreadScheduler::doRun() {
    while (awaitWorkItemsFetched()) {
        m_fetchedWorker->resume();
        m_fetchedWorker = nullptr;
    }
    DEBUG_ASSERT(isStopping());
}
