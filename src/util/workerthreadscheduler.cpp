#include "util/workerthreadscheduler.h"

#include "util/workerthread.h"


WorkerThreadScheduler::WorkerThreadScheduler(
        size_t maxWorkers,
        const QString& name)
        : WorkerThread(name.isEmpty() ? QString("WorkerThreadScheduler") : name),
          m_scheduledWorkers(maxWorkers),
          m_fetchedWorker(nullptr) {
}

bool WorkerThreadScheduler::scheduleWorker(WorkerThread* worker) {
    DEBUG_ASSERT(worker);
    return m_scheduledWorkers.try_push(worker);
}

WorkerThread::FetchWorkResult WorkerThreadScheduler::tryFetchWorkItems() {
    DEBUG_ASSERT(!m_fetchedWorker);
    WorkerThread* worker;
    if (m_scheduledWorkers.pop(&worker)) {
        DEBUG_ASSERT(worker);
        m_fetchedWorker = worker;
        return FetchWorkResult::Ready;
    } else {
        // Suspend the thread after all scheduled workers have
        // have been resumed.
        return FetchWorkResult::Suspend;
    }
}

void WorkerThreadScheduler::doRun() {
    while (waitUntilWorkItemsFetched()) {
        m_fetchedWorker->resume();
        m_fetchedWorker = nullptr;
    }
    DEBUG_ASSERT(isStopping());
}
