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
          m_pause(false),
          m_stop(false) {
}

WorkerThread::~WorkerThread() {
    m_logger.debug() << "Destroying";
    VERIFY_OR_DEBUG_ASSERT(isFinished()) {
        m_logger.warning() << "Waiting for thread to finish";
        stop();
        // The following operation will block the host thread!
        wait();
        DEBUG_ASSERT(isFinished());
    }
}

void WorkerThread::run() {
    if (readStopped()) {
        return;
    }

    const int threadNumber = s_threadCounter.fetch_add(1) + 1;
    const QString threadName =
            m_name.isEmpty() ? QString::number(threadNumber) : QString("%1 #%2").arg(m_name, QString::number(threadNumber));
    QThread::currentThread()->setObjectName(threadName);

    m_logger.debug() << "Running";

    exec();

    m_logger.debug() << "Exiting";

    m_stop.store(true);
}

void WorkerThread::pause() {
    logTrace(m_logger, "Pause");
    m_pause.store(true);
}

void WorkerThread::resume() {
    bool paused = true;
    // Reset value: true -> false
    if (m_pause.compare_exchange_strong(paused, false)) {
        logTrace(m_logger, "Resume");
        // The thread might just be preparing to pause after
        // reading detecting that m_pause was true. To avoid
        // a race condition we need to acquire the mutex that
        // is associated with the wait condition, before
        // signalling the condition. Otherwise the signal
        // of the wait condition might arrive before the
        // thread actually got suspended.
        std::unique_lock<std::mutex> locked(m_sleepMutex);
        wake();
    } else {
        // Just in case, wake up the thread even if it wasn't
        // explicitly paused without locking the mutex. The
        // thread will suspend itself if it is idle.
        wake();
    }
}

void WorkerThread::stop() {
    m_logger.debug() << "Stop";
    m_stop.store(true);
    // Wake up the thread to make sure that the stop flag is
    // detected and the thread commits suicide by exiting the
    // run loop in exec(). Resuming will reset the pause flag
    // to wake up not only an idle but also a paused thread!
    resume();
}

void WorkerThread::whilePaused() {
    DEBUG_ASSERT(QThread::currentThread() == this);
    // The pause flag is always reset after the stop flag has been set,
    // so we don't need to check it separately here.
    if (!m_pause.load()) {
        // Early exit without locking the mutex
        return;
    }
    std::unique_lock<std::mutex> locked(m_sleepMutex);
    whilePaused(&locked);
}

void WorkerThread::whilePaused(std::unique_lock<std::mutex>* locked) {
    DEBUG_ASSERT(locked);
    while (m_pause.load()) {
        logTrace(m_logger, "Suspending while paused");
        m_sleepWaitCond.wait(*locked) ;
        logTrace(m_logger, "Resuming after paused");
    }
}

bool WorkerThread::fetchWorkBlocking() {
    if (readStopped()) {
        // Early exit without locking the mutex
        return false;
    }
    // Keep the mutex locked while idle or paused
    std::unique_lock<std::mutex> locked(m_sleepMutex);
    while (!readStopped()) {
        FetchWorkResult fetchWorkResult = fetchWork();
        switch (fetchWorkResult) {
        case FetchWorkResult::Ready:
            return true;
        case FetchWorkResult::Idle:
            logTrace(m_logger, "Suspending while idle");
            m_sleepWaitCond.wait(locked) ;
            logTrace(m_logger, "Resuming after idle");
            break;
        case FetchWorkResult::Pause:
            m_pause.store(true);
            whilePaused(&locked);
            break;
        }
    }
    return false;
}
