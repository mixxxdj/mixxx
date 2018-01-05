#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

#include <QThread>

#include "util/logger.h"


// A worker thread without an event loop.
//
// This object lives in the creating thread of the host, i.e. does not
// run its own event loop. It does not does not use slots for communication
// with its host which would otherwise still be executed in the host's
// thread.
//
// Signals emitted from the internal worker thread by derived classes
// will queued connections. Communication in the opposite direction is
// accomplished by using lock-free types to avoid locking the host
// thread through priority inversion. Lock-free types might also used
// for any shared state that is read from the host thread after being
// notified about changes.
//
// Derived classes or their owners are responsible to start the thread
// with the desired priority.
class WorkerThread : public QThread {
  public:
    explicit WorkerThread(
            const QString& name = QString());
    // The destructor must be triggered by calling deleteLater() to
    // ensure that the thread has already finished and is not running
    // while destroyed!
    ~WorkerThread() override;

    operator bool() const {
        return !readStopped();
    }

    const QString& name() const {
        return m_name;
    }

    void pause();
    void resume();

    void stop();

  protected:
    void run() final;

    // The internal event loop. Not to be confused with the
    // Qt event loop since the worker thread doesn't has one!
    virtual void exec() = 0;

    // Non-blocking function that determines whether the worker thread
    // is idle (i.e. no new tasks have been scheduled) and should be
    // suspended until resumed or woken up. Returns true as long as the
    // thread should be kept suspended because no work is available.
    // The stop flag does not have to be taken into account here.
    virtual bool readIdle() = 0;

    // Blocks while idle and not stopped. Returns true when the thread
    // should continue processing and false when stopped and the thread
    // should exit asap.
    bool whileIdleAndNotStopped();

    // Blocks the worker thread while the pause flag is set
    void whilePaused();

    void wake() {
        m_sleepWaitCond.notify_one();
    }

    // Non-blocking atomic read of the stop flag
    bool readStopped() const {
        return m_stop.load();
    }

    std::mutex m_sleepMutex;

  private:
    const QString m_name;

    const mixxx::Logger m_logger;

    std::atomic<bool> m_pause;
    std::atomic<bool> m_stop;

    std::condition_variable m_sleepWaitCond;
};
