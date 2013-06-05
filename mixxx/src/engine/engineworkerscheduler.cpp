// engineworkerscheduler.cpp
// Created 6/2/2010 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "engine/engineworker.h"
#include "engine/engineworkerscheduler.h"

EngineWorkerScheduler::EngineWorkerScheduler(QObject* pParent)
        : m_scheduleFIFO(MAX_ENGINE_WORKERS),
          m_bQuit(false) {
    Q_UNUSED(pParent);
    m_workerThreadPool.setMaxThreadCount(ENGINE_WORKER_THREAD_COUNT);
    // A timeout of 1 minute for threads in the pool.
    m_workerThreadPool.setExpiryTimeout(60000);
}

EngineWorkerScheduler::~EngineWorkerScheduler() {
    m_bQuit = true;
    m_waitCondition.wakeAll();
    m_workerThreadPool.waitForDone();
    wait();
}

void EngineWorkerScheduler::bindWorker(EngineWorker* pWorker) {
    pWorker->setScheduler(this);
}

void EngineWorkerScheduler::workerReady(EngineWorker* pWorker) {
    if (pWorker) {
        // If the write fails, we really can't do much since we should not block
        // in this slot. Write the address of the variable pWorker, since it is
        // a 1-element array.
        m_scheduleFIFO.write(&pWorker, 1);
    }
}

void EngineWorkerScheduler::runWorkers() {
    m_waitCondition.wakeAll();
}

void EngineWorkerScheduler::run() {
    while (!m_bQuit) {
        EngineWorker* pWorker = NULL;
        while (m_scheduleFIFO.read(&pWorker, 1) == 1) {
            if (pWorker && !pWorker->isActive()) {
                m_workerThreadPool.start(pWorker);
            }
        }
        m_mutex.lock();
        m_waitCondition.wait(&m_mutex); // unlock mutex and wait
        m_mutex.unlock();
    }
}

