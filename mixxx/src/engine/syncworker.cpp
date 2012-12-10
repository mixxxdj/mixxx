#include <QApplication>

#include "engine/syncworker.h"

#include "controlobject.h"
#include "engine/engineworkerscheduler.h"

SyncWorker::SyncWorker(EngineWorkerScheduler* pScheduler) {
    pScheduler->bindWorker(this);
    installEventFilter(this);
}

SyncWorker::~SyncWorker() {
}

#define MIXXXEVENT_SYNC ((QEvent::Type)(QEvent::User+4))
bool SyncWorker::eventFilter(QObject* o, QEvent* e) {
    if (e && e->type() == MIXXXEVENT_SYNC) {
        ControlObject::sync();
        return true;
    }
    return QObject::eventFilter(o,e);
}

void SyncWorker::run() {
    // Notify the EngineWorkerScheduler that the work we scheduled is starting.
    emit(workStarting(this));

    QApplication::postEvent(this, new QEvent(MIXXXEVENT_SYNC));

    // Notify the EngineWorkerScheduler that the work we did is done.
    emit(workDone(this));
}

void SyncWorker::schedule() {
    emit(workReady(this));
}
