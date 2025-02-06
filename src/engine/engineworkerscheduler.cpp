#include "engine/engineworkerscheduler.h"

#include "engine/engineworker.h"
#include "moc_engineworkerscheduler.cpp"
#include "util/compatibility/qmutex.h"
#include "util/event.h"

EngineWorkerScheduler::EngineWorkerScheduler(QObject* pParent)
        : m_bWakeScheduler(false),
          m_bQuit(false) {
    Q_UNUSED(pParent);
}

EngineWorkerScheduler::~EngineWorkerScheduler() {
    {
        // tell run method to terminate
        const auto lock = lockMutex(&m_mutex);
        m_bQuit = true;
        m_waitCondition.wakeAll();
    }
    // wait for thread to terminate
    wait();
}

void EngineWorkerScheduler::workerReady() {
    m_bWakeScheduler.store(true);
}

void EngineWorkerScheduler::addWorker(EngineWorker* pWorker) {
    DEBUG_ASSERT(pWorker);
    const auto lock = lockMutex(&m_mutex);
    m_workers.push_back(pWorker);
}

void EngineWorkerScheduler::runWorkers() {
    // Wake the scheduler if we have written a worker-ready message to the
    // scheduler. This is called from the callback thread, so we use an
    // atomic and not a mutex.
    if (m_bWakeScheduler.exchange(false)) {
        m_waitCondition.wakeAll();
    }
}

void EngineWorkerScheduler::run() {
    static const QString tag("EngineWorkerScheduler");
    bool quit = false;
    while (!quit) {
        Event::start(tag);
        {
            const auto lock = lockMutex(&m_mutex);
            for(const auto& pWorker: m_workers) {
                pWorker->wakeIfReady();
            }
        }
        Event::end(tag);
        {
            const auto lock = lockMutex(&m_mutex);
            if (!m_bQuit) {
                // Wait for next runWorkers() call
                m_waitCondition.wait(&m_mutex); // unlock mutex and wait
            }
            // copy mutex protected var to local
            quit = m_bQuit;
        }
    }
}
