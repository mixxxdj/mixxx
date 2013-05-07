#ifndef SYNCWORKER_H
#define SYNCWORKER_H

#include <QObject>
#include <QEvent>

#include "engine/engineworker.h"

class EngineWorkerScheduler;

class SyncWorker : public EngineWorker {
    Q_OBJECT
  public:
    explicit SyncWorker(EngineWorkerScheduler* pScheduler);
    virtual ~SyncWorker();

    void run();
    void schedule();

    bool eventFilter(QObject* o, QEvent* e);
};

#endif /* SYNCWORKER_H */
