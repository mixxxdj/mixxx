// engineworkerscheduler.cpp
// Created 6/2/2010 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "engine/engineworker.h"
#include "engine/engineworkerscheduler.h"
#include "util/event.h"

EngineWorkerScheduler::EngineWorkerScheduler(QObject* pParent)
        : m_bWakeScheduler(false),
          m_scheduleFIFO(MAX_ENGINE_WORKERS),
          m_bQuit(false) {
    Q_UNUSED(pParent);
}

EngineWorkerScheduler::~EngineWorkerScheduler() {
    m_mutex.lock();
    m_bQuit = true;
    m_waitCondition.wakeAll();
    m_mutex.unlock();
    wait();
}

void EngineWorkerScheduler::workerReady(EngineWorker* pWorker) {
    if (pWorker) {
        // If the write fails, we really can't do much since we should not block
        // in this slot. Write the address of the variable pWorker, since it is
        // a 1-element array.
        m_scheduleFIFO.write(&pWorker, 1);
        m_bWakeScheduler = true;
    }
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
    while (!m_bQuit) {
        Event::start("EngineWorkerScheduler");
        EngineWorker* pWorker = NULL;
        while (m_scheduleFIFO.read(&pWorker, 1) == 1) {
            if (pWorker) {
                pWorker->wake();
            }
        }
        Event::end("EngineWorkerScheduler");
        m_mutex.lock();
        if (!m_bQuit) {
            m_waitCondition.wait(&m_mutex); // unlock mutex and wait
        }
        m_mutex.unlock();
    }
}
