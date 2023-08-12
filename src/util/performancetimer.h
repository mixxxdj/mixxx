#pragma once
#include <QElapsedTimer>

#include "util/duration.h"

class PerformanceTimer {
  public:
    PerformanceTimer()
            : m_elapsedTimer() {
        // TODO: turn this into a static_assert once Qt enables it.
        DEBUG_ASSERT(QElapsedTimer::clockType() == QElapsedTimer::ClockType::PerformanceCounter);
    }

    void start() {
        m_elapsedTimer.start();
    }

    mixxx::Duration elapsed() const {
        return mixxx::Duration::fromNanos(m_elapsedTimer.nsecsElapsed());
    }

    mixxx::Duration restart() {
        const mixxx::Duration elapsedNs = elapsed();
        m_elapsedTimer.restart();
        return elapsedNs;
    }

    mixxx::Duration difference(const PerformanceTimer& timer) const {
        return timer.elapsed() - elapsed();
    }

    bool running() const {
        return m_elapsedTimer.isValid();
    }

  private:
    QElapsedTimer m_elapsedTimer;
};
