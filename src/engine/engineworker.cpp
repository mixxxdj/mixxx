// engineworker.cpp
// Created 6/2/2010 by RJ Ryan (rryan@mit.edu)

#include "engine/engineworker.h"
#include "engine/engineworkerscheduler.h"

EngineWorker::EngineWorker()
    : m_pScheduler(NULL) {
}

EngineWorker::~EngineWorker() {
}

void EngineWorker::run() {
}

void EngineWorker::setScheduler(EngineWorkerScheduler* pScheduler) {
    m_pScheduler = pScheduler;
    pScheduler->addWorker(this);
}

void EngineWorker::workReady() {
    m_ready = true;
    VERIFY_OR_DEBUG_ASSERT(m_pScheduler) {
        return;     
    }
    m_pScheduler->workerReady();
}

bool EngineWorker::isReady() {
    return m_ready;
}
