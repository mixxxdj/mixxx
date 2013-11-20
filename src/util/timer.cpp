#include "util/timer.h"

Timer::Timer(const QString& key, Stat::ComputeFlags compute)
        : m_key(key),
          m_compute(compute),
          m_running(false) {
}

void Timer::start() {
    m_time.start();
    m_running = true;
}

int Timer::restart(bool report) {
    if (m_running) {
        int nsec = m_time.restart();
        if (report) {
            Stat::track(m_key, Stat::DURATION_NANOSEC, m_compute, nsec);
        }
        return nsec;
    } else {
        start();
        return 0;
    }
}

int Timer::elapsed(bool report) {
    int nsec = m_time.elapsed();
    if (report) {
        Stat::track(m_key, Stat::DURATION_NANOSEC, m_compute, nsec);
    }
    return nsec;
}
