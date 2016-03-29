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

mixxx::Duration Timer::restart(bool report) {
    if (m_running) {
        mixxx::Duration elapsed = m_time.restart();
        if (report) {
            // Ignore the report if it crosses the experiment boundary.
            Experiment::Mode oldMode = Stat::modeFromFlags(m_compute);
            if (oldMode == Experiment::mode()) {
                Stat::track(m_key, Stat::DURATION_NANOSEC, m_compute,
                            elapsed.toIntegerNanos());
            }
        }
        return elapsed;
    } else {
        start();
        return mixxx::Duration::fromNanos(0);
    }
}

mixxx::Duration Timer::elapsed(bool report) {
    mixxx::Duration elapsedTime = m_time.elapsed();
    if (report) {
        // Ignore the report if it crosses the experiment boundary.
        Experiment::Mode oldMode = Stat::modeFromFlags(m_compute);
        if (oldMode == Experiment::mode()) {
            Stat::track(m_key, Stat::DURATION_NANOSEC, m_compute,
                        elapsedTime.toIntegerNanos());
        }
    }
    return elapsedTime;
}


SuspendableTimer::SuspendableTimer(const QString& key,
                                   Stat::ComputeFlags compute)
        : Timer(key, compute) {
}

void SuspendableTimer::start() {
    m_leapTime = mixxx::Duration::fromSeconds(0);
    Timer::start();
}

mixxx::Duration SuspendableTimer::suspend() {
    m_leapTime += m_time.elapsed();
    m_running = false;
    return m_leapTime;
}

void SuspendableTimer::go() {
    Timer::start();
}

mixxx::Duration SuspendableTimer::elapsed(bool report) {
    m_leapTime += m_time.elapsed();
    if (report) {
        // Ignore the report if it crosses the experiment boundary.
        Experiment::Mode oldMode = Stat::modeFromFlags(m_compute);
        if (oldMode == Experiment::mode()) {
            Stat::track(m_key, Stat::DURATION_NANOSEC, m_compute,
                        m_leapTime.toIntegerNanos());
        }
    }
    return m_leapTime;
}
