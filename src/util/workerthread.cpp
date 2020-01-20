#include "util/workerthread.h"

#include "moc_workerthread.cpp"

namespace {

// Enable trace logging only temporary for debugging purposes
// during development!
constexpr bool kEnableTraceLogging = false;

inline
void logTrace(const mixxx::Logger& log, const char* msg) {
    if (kEnableTraceLogging) {
        log.trace() << (msg);
    }
}

std::atomic<int> s_threadCounter(0);

} // anonymous namespace

WorkerThread::WorkerThread(
        const QString& name,
        QThread::Priority priority)
        : m_name(name),
          m_priority(priority),
          m_logger(m_name.isEmpty() ? "WorkerThread" : m_name.toLatin1().constData()),
          m_suspend(false),
          m_stop(false) {
}

WorkerThread::~WorkerThread() {
    m_logger.debug() << "Destroying";
    VERIFY_OR_DEBUG_ASSERT(isFinished()) {
        stop();
        m_logger.warning() << "Waiting until finished";
        // The following operation will block the calling thread!
        wait();
        DEBUG_ASSERT(isFinished());
    }
}

void WorkerThread::deleteAfterFinished() {
    if (!isFinished()) {
        connect(this, &WorkerThread::finished, this, &WorkerThread::deleteLater);
    }
    if (isFinished()) {
        // Already finished or just finished in the meantime. Calling
        // deleteLater() twice is safe, though.
        DEBUG_ASSERT(QThread::currentThread() == thread());
        deleteLater();
    }
}

void WorkerThread::run() {
    if (isStopping()) {
        return;
    }

    const int threadNumber = s_threadCounter.fetch_add(1) + 1;
    const QString threadName =
            m_name.isEmpty() ? QString::number(threadNumber) : QString("%1 #%2").arg(m_name, QString::number(threadNumber));
    setObjectName(threadName);

    if (m_priority != QThread::InheritPriority) {
        m_logger.debug() << "Set priority to: " << m_priority;
        setPriority(m_priority);
    }

    m_logger.debug() << "Running";

    doRun();

    m_logger.debug() << "Exiting";

    m_stop.store(true);
}

void WorkerThread::suspend() {
    DEBUG_ASSERT(QThread::currentThread() != this);
    logTrace(m_logger, "Suspending");
    m_suspend.store(true);
    // The thread will suspend processing and fall asleep the
    // next time it checks m_suspend. If it has already been
    // suspended or is currently sleeping that's fine.
}

void WorkerThread::resume() {
    DEBUG_ASSERT(QThread::currentThread() != this);
    logTrace(m_logger, "Resuming");
    // Reset m_suspend to false to allow the thread to make progress.
    m_suspend.store(false);
    // Wake up the thread so that it is able to check m_suspend and
    // continue processing. To avoid race conditions this needs to
    // be performed unconditionally even if m_suspend was false and has
    // not been modified above! The thread might have checked m_suspend
    // while it was still true. We need to give it the chance to check
    // m_suspend again.
    wake();
}

void WorkerThread::wake() {
    DEBUG_ASSERT(QThread::currentThread() != this);
    logTrace(m_logger, "Waking up");
    // Suspend the calling thread until the worker thread actually
    // is sleeping, i.e. is blocking on m_sleepWaitCond. Otherwise
    // the worker thread might invoke m_sleepWaitCond.wait(locked)
    // and become sleeping just after the following notification
    // has been signaled. In this case the signal would be lost
    // and the worker thread would remain sleeping forever!
    std::unique_lock<std::mutex> locked(m_sleepMutex);
    m_sleepWaitCond.notify_one();
}

void WorkerThread::stop() {
    DEBUG_ASSERT(QThread::currentThread() != this);
    logTrace(m_logger, "Stopping");
    m_stop.store(true);
    // Wake up the thread to make sure that the stop flag is
    // detected to exit the run loop. Resuming will reset the
    // suspend flag to wake up not only idle but also a suspended
    // threads! Otherwise suspended threads would sleep forever
    // and never notice that they have been stopped.
    resume();
}

void WorkerThread::sleepWhileSuspended() {
    DEBUG_ASSERT(QThread::currentThread() == this);
    // The suspend flag is always reset after the stop flag has been set,
    // so we don't need to check it separately here.
    if (!m_suspend.load()) {
        // Early exit without locking the mutex
        return;
    }
    std::unique_lock<std::mutex> locked(m_sleepMutex);
    while (m_suspend.load()) {
        logTrace(m_logger, "Sleeping while suspended");
        m_sleepWaitCond.wait(locked) ;
        logTrace(m_logger, "Continuing after sleeping while suspended");
    }
}

bool WorkerThread::awaitWorkItemsFetched() {
    if (isStopping()) {
        // Early exit without locking the mutex
        return false;
    }
    // Keep the mutex locked while idle or suspended
    std::unique_lock<std::mutex> locked(m_sleepMutex);
    while (!isStopping()) {
        TryFetchWorkItemsResult fetchWorkResult = tryFetchWorkItems();
        switch (fetchWorkResult) {
        case TryFetchWorkItemsResult::Ready:
            logTrace(m_logger, "Work items fetched and ready");
            return true;
        case TryFetchWorkItemsResult::Idle:
            logTrace(m_logger, "Sleeping while idle");
            m_sleepWaitCond.wait(locked) ;
            logTrace(m_logger, "Continuing after slept while idle");
            break;
        }
    }
    return false;
}
