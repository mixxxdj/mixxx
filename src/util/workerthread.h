#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

#include <QThread>

#include "util/logger.h"


/// A worker thread without an event loop.
///
/// This object lives in the creating thread of the host, i.e. does not
/// run its own event loop. It does not use slots for communication
/// with its host which would otherwise still be executed in the host's
/// thread.
///
/// Signals emitted from the internal worker thread by derived classes
/// will queued connections. Communication in the opposite direction is
/// accomplished by using lock-free types to avoid locking the host
/// thread through priority inversion. Lock-free types might also used
/// for any shared state that is read from the host thread after being
/// notified about changes.
///
/// Derived classes or their owners are responsible to start the thread
/// with the desired priority.
class WorkerThread : public QThread {
    Q_OBJECT

  public:
    explicit WorkerThread(
            const QString& name = QString(),
            const QThread::Priority priority = QThread::InheritPriority);
    /// The destructor must be triggered by calling deleteLater() to
    /// ensure that the thread has already finished and is not running
    /// while destroyed! Connect finished() to deleteAfter() and then
    /// call stop() on the running worker thread explicitly to trigger
    /// the destruction. Use deleteAfterFinished() for this purpose.
    ~WorkerThread() override;

    void deleteAfterFinished();

    const QString& name() const {
        return m_name;
    }

    /// Commands the thread to suspend itself asap.
    ///
    /// Must not be invoked from the worker thread itself to
    /// avoid race conditions!
    void suspend();

    /// Resumes a suspended thread by waking it up.
    ///
    /// Must not be invoked from the worker thread itself to
    /// avoid race conditions!
    void resume();

    /// Wakes up a sleeping thread. If the thread has been suspended
    /// it will fall asleep again. A suspended thread needs to be
    /// resumed.
    ///
    /// Must not be invoked from the worker thread itself to
    /// avoid race conditions!
    void wake();

    /// Commands the thread to stop asap. This action is irreversible,
    /// i.e. the thread cannot be restarted once it has been stopped.
    ///
    /// Must not be invoked from the worker thread itself to
    /// avoid race conditions!
    void stop();

    /// Non-blocking atomic read of the stop flag which indicates that
    /// the thread is stopping, i.e. it will soon exit or already has
    /// exited the run loop.
    bool isStopping() const {
        return m_stop.load();
    }

  protected:
    void run() final;

    /// The internal run loop. Not to be confused with the Qt event
    /// loop since the worker thread doesn't have one!
    /// An implementation may exit this loop after all work is done,
    /// which in turn exits and terminates the thread. The loop should
    /// also be left asap when isStopping() returns true. This condition
    /// should be checked repeatedly during execution of the loop and
    /// especially before starting any expensive subtasks.
    virtual void doRun() = 0;

    enum class TryFetchWorkItemsResult {
        Ready,
        Idle,
    };

    /// Non-blocking function that determines whether the worker thread
    /// is idle (i.e. no new tasks have been scheduled) and should be
    /// either suspended until resumed or put to sleep until woken up.
    ///
    /// Implementing classes are able to control what to do if no more
    /// work is currently available. Returning TryFetchWorkItemsResult::Idle
    /// preserves the current suspend state and lets the thread sleep
    /// until wake() is called.
    ///
    /// Implementing classes are responsible for storing the fetched
    /// work items internally for later processing during
    /// doRun().
    ///
    /// The stop flag does not have to be checked when entering this function,
    /// because it has already been checked just before the invocation. Though
    /// the fetch operation may check again before starting any expensive
    /// internal subtask.
    virtual TryFetchWorkItemsResult tryFetchWorkItems() = 0;

    /// Blocks while idle and not stopped. Returns true when new work items
    /// for processing have been fetched and false if the thread has been
    /// stopped while waiting.
    bool awaitWorkItemsFetched();

    /// Blocks the worker thread while the suspend flag is set.
    ///
    /// Worker threads should invoke this yield-like method periodically.
    /// Especially before starting any computational intensive work that
    /// is expected to take some time.
    ///
    /// After returning from this function worker threads should also
    /// check isStopping() and exit if they have been asked to stop.
    ///
    /// This function must not be called from tryFetchWorkItems()
    /// to avoid a deadlock on the non-recursive mutex!
    void sleepWhileSuspended();

  private:
    const QString m_name;
    const QThread::Priority m_priority;

    const mixxx::Logger m_logger;

    std::atomic<bool> m_suspend;
    std::atomic<bool> m_stop;

    std::mutex m_sleepMutex;
    std::condition_variable m_sleepWaitCond;
};
