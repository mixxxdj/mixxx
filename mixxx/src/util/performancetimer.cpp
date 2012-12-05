#include "util/performancetimer.h"

#if defined(__APPLE__)
#include <CoreServices/CoreServices.h>
#include <mach/mach_time.h>

quint64 durationToNanoseconds(quint64 duration) {
    // See http://developer.apple.com/library/mac/#qa/qa1398/_index.html
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

#else defined(__WINDOWS__)

#endif

