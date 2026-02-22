#include "util/timer.h"

#include "util/experiment.h"
#include "util/time.h"

Timer::Timer(QString key, Stat::ComputeFlags compute)
        : m_key(std::move(key)),
          m_compute(Stat::experimentFlags(compute)) {
}

void Timer::start() {
    m_time.start();
}

mixxx::Duration Timer::restart(bool report) {
    if (m_time.running()) {
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
