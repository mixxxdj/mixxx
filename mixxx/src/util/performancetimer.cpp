#include "util/performancetimer.h"

#ifdef __APPLE__
#include <CoreServices/CoreServices.h>
#include <mach/mach_time.h>

quint64 durationToNanoseconds(quint64 duration) {
    // Convert to nanoseconds.
    // Have to do some pointer fun because AbsoluteToNanoseconds
    // works in terms of UnsignedWide, which is a structure rather
    // than a proper 64-bit integer.

    Nanoseconds nsec = AbsoluteToNanoseconds(*(AbsoluteTime*)&duration);
    return *(quint64*)&nsec;
}

PerformanceTimer::PerformanceTimer()
        : m_running(false),
          m_start(0) {
}

PerformanceTimer::~PerformanceTimer() {
}

void PerformanceTimer::start() {
    m_running = true;
    m_start = mach_absolute_time();
}

quint64 PerformanceTimer::restart() {
    if (!m_running) {
        start();
        return 0;
    }
    quint64 now = mach_absolute_time();
    quint64 duration = now - m_start;
    m_start = now;
    return durationToNanoseconds(duration);
}

quint64 PerformanceTimer::elapsed() {
    if (!m_running) {
        return 0;
    }
    quint64 now = mach_absolute_time();
    quint64 duration = now - m_start;
    return durationToNanoseconds(duration);
}

#endif

