// engineworkerscheduler.cpp
// Created 6/2/2010 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "engine/engineworker.h"
#include "engine/engineworkerscheduler.h"

EngineWorkerScheduler::EngineWorkerScheduler(QObject* pParent)
        : QThreadPool(pParent) {

    connect(&m_readyMapper, SIGNAL(mapped(QObject*)),
            this, SLOT(workerReady(QObject*)));
    connect(&m_startedMapper, SIGNAL(mapped(QObject*)),
            this, SLOT(workerStarted(QObject*)));
    connect(&m_finishedMapper, SIGNAL(mapped(QObject*)),
            this, SLOT(workerFinished(QObject*)));
}

EngineWorkerScheduler::~EngineWorkerScheduler() {

}

void EngineWorkerScheduler::bindWorker(EngineWorker* pWorker) {
    m_readyMapper.setMapping(pWorker, pWorker);
    m_startedMapper.setMapping(pWorker, pWorker);
    m_finishedMapper.setMapping(pWorker, pWorker);

    connect(pWorker, SIGNAL(workReady()),
            &m_readyMapper, SLOT(map()));
    connect(pWorker, SIGNAL(workStarting()),
            &m_startedMapper, SLOT(map()));
    connect(pWorker, SIGNAL(workDone()),
            &m_finishedMapper, SLOT(map()));
}

void EngineWorkerScheduler::workerReady(QObject* pObject) {
    EngineWorker* pWorker = dynamic_cast<EngineWorker*>(pObject);
    Q_ASSERT(pWorker);
    m_scheduledWorkers.insert(pWorker);
}

void EngineWorkerScheduler::workerStarted(QObject* pObject) {
    EngineWorker* pWorker = dynamic_cast<EngineWorker*>(pObject);
    Q_ASSERT(pWorker);
    m_activeWorkers.insert(pWorker);
}

void EngineWorkerScheduler::workerFinished(QObject* pObject) {
    EngineWorker* pWorker = dynamic_cast<EngineWorker*>(pObject);
    Q_ASSERT(pWorker);
    m_activeWorkers.remove(pWorker);
}


void EngineWorkerScheduler::runWorkers() {
    QMutableSetIterator<EngineWorker*> it(m_scheduledWorkers);
    while (it.hasNext()) {
        EngineWorker* pWorker = it.next();
        it.remove();
        start(pWorker);
    }
}

