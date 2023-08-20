#pragma once
#include <QElapsedTimer>

#include "util/duration.h"

class PerformanceTimer {
  public:
    PerformanceTimer()
            : m_elapsedTimer() {
    }

    // call this once at startup to ensure that the QElapsedTimer is monotonic
    static bool debugEnsureClockIsMonotonic() {
        DEBUG_ASSERT(QElapsedTimer::isMonotonic());
        return QElapsedTimer::isMonotonic();
    };

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
