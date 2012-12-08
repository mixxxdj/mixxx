#include <QTime>

#if defined(__APPLE__)
#include <mach/mach_time.h>
#elif defined(__WINDOWS__)
#include <windows.h>
#endif

#include "util/performancetimer.h"

PerformanceTimer::PerformanceTimer()
        : m_running(false),
          m_start(0),
          m_freq_numerator(1),
          m_freq_denominator(1),
          m_time(NULL) {

#if defined(__APPLE__)
    // See http://developer.apple.com/library/mac/#qa/qa1398/_index.html
    static mach_timebase_info_data_t sTimebaseInfo = { 0, 0 };
    if (sTimebaseInfo.denom == 0) {
        mach_timebase_info(&sTimebaseInfo);
    }
    m_freq_numerator = sTimebaseInfo.numer;
    m_freq_denominator = sTimebaseInfo.denom;
#elif defined(__WINDOWS__)
    static LARGE_INTEGER frequency = { 0, 0 };
    if (frequency.QuadPart == 0) {
        if (QueryPerformanceFrequency(&frequency)) {
            // Dividing a delta by the frequency produces seconds so multiplying by
            // 1e9 produces nanoseconds.
            m_freq_numerator = 1e9;
            m_freq_denominator = frequency.QuadPart;
        }
    }
#else
    m_time = new QTime();
#endif
}

PerformanceTimer::~PerformanceTimer() {
    delete m_time;
}

void PerformanceTimer::start() {
    if (m_time) {
        m_time->start();
        return;
    }

    m_running = true;
    m_start = count();
}

quint64 PerformanceTimer::restart() {
    if (m_time) {
        return m_time->restart() * 1e6;
    }
    if (!m_running) {
        start();
        return 0;
    }
    quint64 now = count();
    quint64 duration = now - m_start;
    m_start = now;
    return countDeltaToNanoseconds(duration);
}

quint64 PerformanceTimer::elapsed() {
    if (m_time) {
        return m_time->elapsed() * 1e6;
    }
    if (!m_running) {
        return 0;
    }
    quint64 now = count();
    quint64 duration = now - m_start;
    return countDeltaToNanoseconds(duration);
}

quint64 PerformanceTimer::countDeltaToNanoseconds(quint64 delta) const {
    // TODO(rryan): Look for overflow.
    return delta * m_freq_numerator / m_freq_denominator;
}

#if defined(__APPLE__)
quint64 PerformanceTimer::count() const {
    return mach_absolute_time();
}
#elif defined(__WINDOWS__)
quint64 PerformanceTimer::count() const {
    LARGE_INTEGER count;
    if (QueryPerformanceCounter(&count)) {
        return count.QuadPart;
    }
    return 0;
}
#else
quint64 PerformanceTimer::count() const {
    return 0;
}
#endif


