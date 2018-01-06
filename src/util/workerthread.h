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
    Q_OBJECT

  public:
    explicit WorkerThread(
            const QString& name = QString());
    // The destructor must be triggered by calling deleteLater() to
    // ensure that the thread has already finished and is not running
    // while destroyed! Connect finished() to deleteAfter() and then
    // call stop() on the running worker thread explicitly to trigger
    // the destruction. Use destroy() for this purpose (see below).
    ~WorkerThread() override;

    void deleteAfterFinished();

    operator bool() const {
        return !readStopped();
    }

    const QString& name() const {
        return m_name;
    }

    // Commands the thread to suspend itself asap.
    void suspend();

    // Resumes a suspended thread by waking it up.
    void resume();

    // Wakes up a sleeping thread. If the thread has been suspended
    // it will fall asleep again. A suspended thread needs to be
    // resumed.
    void wake();

    // Commands the thread to stop asap.
    void stop();

  protected:
    void run() final;

    // The internal event loop. Not to be confused with the
    // Qt event loop since the worker thread doesn't has one!
    virtual void exec() = 0;

    enum class FetchWorkResult {
        Ready,
        Idle,
        Suspend,
    };

    // Non-blocking function that determines whether the worker thread
    // is idle (i.e. no new tasks have been scheduled) and should be
    // suspended until resumed or woken up. Returns true as long as the
    // thread should be kept suspended because no work is available.
    // The stop flag does not have to be taken into account here.
    virtual FetchWorkResult fetchWork() = 0;

    // Blocks while idle and not stopped. Returns true when the thread
    // should continue processing and false when stopped and the thread
    // should exit asap.
    bool fetchWorkBlocking();

    // Blocks the worker thread while the suspend flag is set.
    // This function must not be called while idle which could
    // block on the non-recursive mutex twice!
    void whileSuspended();

    // Non-blocking atomic read of the stop flag
    bool readStopped() const {
        return m_stop.load();
    }

  private:
    void whileSuspended(std::unique_lock<std::mutex>* locked);

    const QString m_name;

    const mixxx::Logger m_logger;

    std::atomic<bool> m_suspend;
    std::atomic<bool> m_stop;

    std::mutex m_sleepMutex;
    std::condition_variable m_sleepWaitCond;
};
