
#include "util/threadcputimer.h"

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


////////////////////////////// Unix //////////////////////////////
#if defined(Q_OS_UNIX)

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
#else
    Q_UNUSED(threadCpuTimeChecked);
    Q_UNUSED(threadCpuTimeAvailable);
    Q_UNUSED(unixCheckClockType);
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

////////////////////////////// Default //////////////////////////////
#else

// default implementation (no hi-perf timer) does nothing
void ThreadCpuTimer::start()
{
}

qint64 ThreadCpuTimer::elapsed() const
{
    return 0;
}

qint64 ThreadCpuTimer::restart()
{
    return 0;
}

#endif
