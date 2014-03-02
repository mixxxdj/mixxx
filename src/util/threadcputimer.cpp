
#include "threadcputimer.h"

#if defined(Q_OS_MAC)
#include <sys/time.h>
#include <unistd.h>
#include <mach/mach_time.h>
#elif defined(Q_OS_SYMBIAN)
#include <e32std.h>
#include <sys/time.h>
#include <hal.h>
#include <hal_data.h>
#elif defined(Q_OS_UNIX)
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#elif defined(Q_OS_WIN)
#include <windows.h>
#endif

// mac/unix code heavily copied from QElapsedTimer


////////////////////////////// Mac //////////////////////////////
#if defined(Q_OS_MAC)

static mach_timebase_info_data_t info = {0,0};
static qint64 absoluteToNSecs(qint64 cpuTime)
{
    if (info.denom == 0)
        mach_timebase_info(&info);
    qint64 nsecs = cpuTime * info.numer / info.denom;
    return nsecs;
}

void PerformanceTimer::start()
{
    t1 = mach_absolute_time();
}

qint64 PerformanceTimer::elapsed() const
{
    uint64_t cpu_time = mach_absolute_time();
    return absoluteToNSecs(cpu_time - t1);
}

qint64 PerformanceTimer::restart()
{
    qint64 start;
    start = t1;
    t1 = mach_absolute_time();
    return absoluteToNSecs(t1-start);
}

qint64 PerformanceTimer::difference(PerformanceTimer* timer)
{
    return absoluteToNSecs(t1 - timer->t1);
}

////////////////////////////// Symbian //////////////////////////////
#elif defined(Q_OS_SYMBIAN)

static qint64 getTimeFromTick(quint64 elapsed)
{
    static TInt freq = 0;
    if (!freq)
        HAL::Get(HALData::EFastCounterFrequency, freq);

    return (elapsed * 1000000000) / freq;
}

void PerformanceTimer::start()
{
    t1 = User::FastCounter();
}

qint64 PerformanceTimer::elapsed() const
{
    return getTimeFromTick(User::FastCounter() - t1);
}

qint64 PerformanceTimer::restart()
{
    qint64 start;
    start = t1;
    t1 = User::FastCounter();
    return getTimeFromTick(t1 - start);
}

qint64 PerformanceTimer::difference(PerformanceTimer* timer)
{
    return getTimeFromTick(t1 - timer->t1);
}

////////////////////////////// Unix //////////////////////////////
#elif defined(Q_OS_UNIX)

#if (_POSIX_THREAD_CPUTIME-0 != 0)
static const bool threadCpuTimeChecked = true;
static const bool threadCpuTimeAvailable = _POSIX_THREAD_CPUTIME > 0;
#else
static int threadCpuTimeChecked = false;
static int threadCpuTimeAvailable = false;
#endif

#ifdef Q_CC_GNU
# define is_likely(x) __builtin_expect((x), 1)
#else
# define is_likely(x) (x)
#endif
#define load_acquire(x) ((volatile const int&)(x))
#define store_release(x,v) ((volatile int&)(x) = (v))

static void unixCheckClockType()
{
#if (_POSIX_THREAD_CPUTIME-0 == 0)
    if (is_likely(load_acquire(threadCpuTimeChecked)))
        return;

# if defined(_SC_THREAD_CPUTIME)
    // detect if the system support monotonic timers
    long x = sysconf(_SC_THREAD_CPUTIME);
    store_release(threadCpuTimeAvailable, x >= 200112L);
# endif

    store_release(threadCpuTimeChecked, true);
#endif
}

static inline void do_gettime(qint64 *sec, qint64 *frac)
{
#if (_POSIX_MONOTONIC_CLOCK-0 >= 0)
    unixCheckClockType();
    if (is_likely(threadCpuTimeAvailable)) {
        timespec ts;
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
        *sec = ts.tv_sec;
        *frac = ts.tv_nsec;
        return;
    }
#endif
    *sec = 0;
    *frac = 0;
}

void ThreadCpuTimer::start()
{
    do_gettime(&t1, &t2);
}

qint64 ThreadCpuTimer::elapsed() const
{
    qint64 sec, frac;
    do_gettime(&sec, &frac);
    sec = sec - t1;
    frac = frac - t2;

    return sec * Q_INT64_C(1000000000) + frac;
}

qint64 ThreadCpuTimer::restart()
{
    qint64 sec, frac;
    sec = t1;
    frac = t2;
    do_gettime(&t1, &t2);
    sec = t1 - sec;
    frac = t2 - frac;
    return sec * Q_INT64_C(1000000000) + frac;
}

////////////////////////////// Windows //////////////////////////////
#elif defined(Q_OS_WIN)

static qint64 getTimeFromTick(quint64 elapsed)
{
    static LARGE_INTEGER freq = {{ 0, 0 }};
    if (!freq.QuadPart)
        QueryPerformanceFrequency(&freq);
    return 1000000000 * elapsed / freq.QuadPart;
}

void PerformanceTimer::start()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    t1 = li.QuadPart;
}

qint64 PerformanceTimer::elapsed() const
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return getTimeFromTick(li.QuadPart - t1);
}

qint64 PerformanceTimer::restart()
{
    LARGE_INTEGER li;
    qint64 start;
    start = t1;
    QueryPerformanceCounter(&li);
    t1 = li.QuadPart;
    return getTimeFromTick(t1 - start);
}

////////////////////////////// Default //////////////////////////////
#else

// default implementation (no hi-perf timer) does nothing
void PerformanceTimer::start()
{
}

qint64 PerformanceTimer::elapsed() const
{
    return 0;
}

qint64 PerformanceTimer::restart() const
{
    return 0;
}

qint64 PerformanceTimer::difference(PerformanceTimer* timer)
{
    return 0;
}

#endif
