// engineworker.h
// Created 6/2/2010 by RJ Ryan (rryan@mit.edu)

#ifndef ENGINEWORKER_H
#define ENGINEWORKER_H

#include <QAtomicInt>
#include <QObject>
#include <QRunnable>

// EngineWorker is an interface for running background processing work when the
// audio callback is not active. While the audio callback is active, an
// EngineWorker can emit its workReady signal, and an EngineWorkerManager will
// schedule it for running after the audio callback has completed.

class EngineWorkerScheduler;

class EngineWorker : public QObject, public QRunnable {
    Q_OBJECT
  public:
    EngineWorker();
    virtual ~EngineWorker();

    virtual void run();

    // Thread-safe, sets whether this EngineWorker is active.
    inline void setActive(bool bActive) {
        m_isActive = bActive;
    }

    // Thread-safe, returns true if this EngineWorker is active.
    inline bool isActive() const {
        return m_isActive > 0;
    }

    void setScheduler(EngineWorkerScheduler* pScheduler);
    void workReady();

  private:
    EngineWorkerScheduler* m_pScheduler;
    QAtomicInt m_isActive;
};

#endif /* ENGINEWORKER_H */
