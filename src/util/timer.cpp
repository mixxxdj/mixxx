#include "util/timer.h"
#include "util/experiment.h"

Timer::Timer(const QString& key, Stat::ComputeFlags compute)
        : m_key(key),
          m_compute(Stat::experimentFlags(compute)),
          m_running(false) {
}

void Timer::start() {
    m_running = true;
    m_time.start();
}

int Timer::restart(bool report) {
    if (m_running) {
        int nsec = m_time.restart();
        if (report) {
            // Ignore the report if it crosses the experiment boundary.
            Experiment::Mode oldMode = Stat::modeFromFlags(m_compute);
            if (oldMode == Experiment::mode()) {
                Stat::track(m_key, Stat::DURATION_NANOSEC, m_compute, nsec);
            }
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
        // Ignore the report if it crosses the experiment boundary.
        Experiment::Mode oldMode = Stat::modeFromFlags(m_compute);
        if (oldMode == Experiment::mode()) {
            Stat::track(m_key, Stat::DURATION_NANOSEC, m_compute, nsec);
        }
    }
    return nsec;
}


SuspendableTimer::SuspendableTimer(const QString& key,
                                   Stat::ComputeFlags compute)
        : Timer(key, compute),
          m_leapTime(0) {
}

void SuspendableTimer::start() {
    m_leapTime = 0;
    Timer::start();
}

int SuspendableTimer::suspend() {
    m_leapTime += m_time.elapsed();
    m_running = false;
    return m_leapTime;
}

void SuspendableTimer::go() {
    Timer::start();
}

int SuspendableTimer::elapsed(bool report) {
    m_leapTime += m_time.elapsed();
    if (report) {
        // Ignore the report if it crosses the experiment boundary.
        Experiment::Mode oldMode = Stat::modeFromFlags(m_compute);
        if (oldMode == Experiment::mode()) {
            Stat::track(m_key, Stat::DURATION_NANOSEC, m_compute, m_leapTime);
        }
    }
    return m_leapTime;
}
