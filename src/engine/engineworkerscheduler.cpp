#include "engine/engineworkerscheduler.h"

#include <QtDebug>

#include "engine/engineworker.h"
#include "moc_engineworkerscheduler.cpp"
#include "util/event.h"

EngineWorkerScheduler::EngineWorkerScheduler(QObject* pParent)
        : m_bWakeScheduler(false),
          m_bQuit(false) {
    Q_UNUSED(pParent);
}

EngineWorkerScheduler::~EngineWorkerScheduler() {
    m_bQuit = true;
    m_waitCondition.wakeAll();
    wait();
}

void EngineWorkerScheduler::workerReady() {
    m_bWakeScheduler = true;
}

void EngineWorkerScheduler::addWorker(EngineWorker* pWorker) {
    DEBUG_ASSERT(pWorker);
    QMutexLocker locker(&m_mutex);
    m_workers.push_back(pWorker);
}

void EngineWorkerScheduler::runWorkers() {
    // Wake the scheduler if we have written a worker-ready message to the
    // scheduler. There is no race condition in accessing this boolean because
    // both workerReady and runWorkers are called from the callback thread.
    if (m_bWakeScheduler) {
        m_bWakeScheduler = false;
        m_waitCondition.wakeAll();
    }
}

void EngineWorkerScheduler::run() {
    static const QString tag("EngineWorkerScheduler");
    while (!m_bQuit) {
        Event::start(tag);
        {
            QMutexLocker lock(&m_mutex);
            for(const auto& pWorker: m_workers) {
                pWorker->wakeIfReady();
            }
        }
        Event::end(tag);
        {
            QMutexLocker lock(&m_mutex);
            if (!m_bQuit) {
                // Wait for next runWorkers() call
                m_waitCondition.wait(&m_mutex); // unlock mutex and wait
            }
        }
    }
}
