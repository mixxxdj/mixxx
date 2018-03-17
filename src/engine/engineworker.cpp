// engineworker.cpp
// Created 6/2/2010 by RJ Ryan (rryan@mit.edu)

#include "engine/engineworker.h"
#include "engine/engineworkerscheduler.h"

EngineWorker::EngineWorker()
    : m_pScheduler(nullptr) {
    m_notReady.test_and_set();
}

EngineWorker::~EngineWorker() {
}

void EngineWorker::run() {
}

void EngineWorker::setScheduler(EngineWorkerScheduler* pScheduler) {
    DEBUG_ASSERT(m_pScheduler == nullptr);
    m_pScheduler = pScheduler;
    pScheduler->addWorker(this);
}

void EngineWorker::workReady() {
    m_notReady.clear();
    VERIFY_OR_DEBUG_ASSERT(m_pScheduler) {
        return;     
    }
    m_pScheduler->workerReady();
}

void EngineWorker::wakeIfReady() {
    if (!m_notReady.test_and_set()) {
        m_semaRun.release();
    }
}
