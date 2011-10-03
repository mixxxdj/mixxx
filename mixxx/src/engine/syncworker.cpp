#include "engine/syncworker.h"

#include "controlobject.h"
#include "engine/engineworkerscheduler.h"

SyncWorker::SyncWorker(EngineWorkerScheduler* pScheduler) {
    pScheduler->bindWorker(this);
}

SyncWorker::~SyncWorker() {
}

void SyncWorker::run() {
    // Notify the EngineWorkerScheduler that the work we scheduled is starting.
    emit(workStarting(this));

    ControlObject::sync();

    // Notify the EngineWorkerScheduler that the work we did is done.
    emit(workDone(this));
}

void SyncWorker::schedule() {
    emit(workReady(this));
}
