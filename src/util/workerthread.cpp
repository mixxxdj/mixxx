#include "util/workerthread.h"


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
        const QString& name)
        : m_name(name),
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
    QThread::currentThread()->setObjectName(threadName);

    m_logger.debug() << "Running";

    doRun();

    m_logger.debug() << "Exiting";

    m_stop.store(true);
}

void WorkerThread::suspend() {
    logTrace(m_logger, "Suspending");
    m_suspend.store(true);
}

void WorkerThread::resume() {
    bool suspended = true;
    // Reset value: true -> false
    if (m_suspend.compare_exchange_strong(suspended, false)) {
        logTrace(m_logger, "Resuming");
        // The thread might just be preparing to suspend after
        // loading and detecting that m_suspend was true. To avoid
        // a race condition we need to acquire the mutex that
        // is associated with the wait condition, before
        // signalling the condition. Otherwise the signal
        // of the wait condition might arrive before the
        // thread actually got suspended.
        std::unique_lock<std::mutex> locked(m_sleepMutex);
        wake();
    } else {
        // Just in case, wake up the thread even if it wasn't
        // explicitly suspended without locking the mutex. The
        // thread will suspend itself if it is idle.
        wake();
    }
}

void WorkerThread::wake() {
    m_logger.debug() << "Waking up";
    m_sleepWaitCond.notify_one();
}

void WorkerThread::stop() {
    m_logger.debug() << "Stopping";
    m_stop.store(true);
    // Wake up the thread to make sure that the stop flag is
    // detected and the thread commits suicide by exiting the
    // run loop in exec(). Resuming will reset the suspend flag
    // to wake up not only an idle but also a suspended thread!
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
    sleepWhileSuspended(&locked);
}

void WorkerThread::sleepWhileSuspended(std::unique_lock<std::mutex>* locked) {
    DEBUG_ASSERT(locked);
    while (m_suspend.load()) {
        logTrace(m_logger, "Sleeping while suspended");
        m_sleepWaitCond.wait(*locked) ;
        logTrace(m_logger, "Continuing after sleeping while suspended");
    }
}

bool WorkerThread::waitUntilWorkItemsFetched() {
    if (isStopping()) {
        // Early exit without locking the mutex
        return false;
    }
    // Keep the mutex locked while idle or suspended
    std::unique_lock<std::mutex> locked(m_sleepMutex);
    while (!isStopping()) {
        FetchWorkResult fetchWorkResult = tryFetchWorkItems();
        switch (fetchWorkResult) {
        case FetchWorkResult::Ready:
            logTrace(m_logger, "Work items fetched and ready");
            return true;
        case FetchWorkResult::Idle:
            logTrace(m_logger, "Sleeping while idle");
            m_sleepWaitCond.wait(locked) ;
            logTrace(m_logger, "Continuing after slept while idle");
            break;
        case FetchWorkResult::Suspend:
            logTrace(m_logger, "Suspending while idle");
            suspend();
            sleepWhileSuspended(&locked);
            logTrace(m_logger, "Continuing after suspended while idle");
            break;
        case FetchWorkResult::Stop:
            logTrace(m_logger, "Stopping after trying to fetch work items");
            stop();
            break;
        }
    }
    return false;
}
