// engineworkerscheduler.cpp
// Created 6/2/2010 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>
#include <QMutexLocker>

#include "engine/engineworker.h"
#include "engine/engineworkerscheduler.h"

EngineWorkerScheduler::EngineWorkerScheduler(QObject* pParent)
        : QThreadPool(pParent) {
}

EngineWorkerScheduler::~EngineWorkerScheduler() {

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
        QMutexLocker locker(&m_mutex);
        m_scheduledWorkers.insert(pWorker);
    }
}

void EngineWorkerScheduler::workerStarted(EngineWorker* pWorker) {
}

void EngineWorkerScheduler::workerFinished(EngineWorker* pWorker) {
}


void EngineWorkerScheduler::runWorkers() {
    QMutexLocker locker(&m_mutex);
    QMutableSetIterator<EngineWorker*> it(m_scheduledWorkers);
    while (it.hasNext()) {
        EngineWorker* pWorker = it.next();
        it.remove();
        start(pWorker);
    }
}

