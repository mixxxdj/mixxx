// engineworkerscheduler.cpp
// Created 6/2/2010 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>
#include <QMutexLocker>

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
}

void EngineWorkerScheduler::bindWorker(EngineWorker* pWorker) {
    connect(pWorker, SIGNAL(workReady(EngineWorker*)),
            this, SLOT(workerReady(EngineWorker*)),
            Qt::DirectConnection);
    connect(pWorker, SIGNAL(workStarting(EngineWorker*)),
            this, SLOT(workerStarted(EngineWorker*)),
            Qt::DirectConnection);
    connect(pWorker, SIGNAL(workDone(EngineWorker*)),
            this, SLOT(workerFinished(EngineWorker*)),
            Qt::DirectConnection);
}

void EngineWorkerScheduler::workerReady(EngineWorker* pWorker) {
    if (pWorker) {
        // If the write fails, we really can't do much since we should not block
        // in this slot. Write the address of the variable pWorker, since it is
        // a 1-element array.
        m_scheduleFIFO.write(&pWorker, 1);
    }
}

void EngineWorkerScheduler::workerStarted(EngineWorker* pWorker) {
    Q_UNUSED(pWorker);
}

void EngineWorkerScheduler::workerFinished(EngineWorker* pWorker) {
    QMutexLocker locker(&m_mutex);
    m_activeWorkers.remove(pWorker);
}

void EngineWorkerScheduler::runWorkers() {
    m_waitCondition.wakeAll();
}

void EngineWorkerScheduler::run() {
    while (!m_bQuit) {
        m_mutex.lock();
        EngineWorker* pWorker = NULL;
        while (m_scheduleFIFO.read(&pWorker, 1) == 1) {
            if (pWorker && !m_activeWorkers.contains(pWorker)) {
                m_activeWorkers.insert(pWorker);
                m_workerThreadPool.start(pWorker);
            }
        }
        m_waitCondition.wait(&m_mutex); // unlock mutex and wait
        m_mutex.unlock();
    }
}

