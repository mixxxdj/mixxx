#pragma once
#include <chrono>

#include "util/duration.h"

class PerformanceTimer {
  public:
    // note that the resolution of std::chrono::steady_clock is not guaranteed
    // to be high resolution, but it is guaranteed to be monotonic.
    // However, on all major platforms, it is high resolution enough.
    using ClockT = std::chrono::steady_clock;
    PerformanceTimer()
            : m_startTime(kStoppedTimerValue){};

    void start() {
        m_startTime = ClockT::now();
    };

    mixxx::Duration elapsed() const {
        return mixxx::Duration::fromStdDuration(ClockT::now() - m_startTime);
    };
    mixxx::Duration restart() {
        const ClockT::time_point now = ClockT::now();
        const auto dur = mixxx::Duration::fromStdDuration(now - m_startTime);
        m_startTime = now;
        return dur;
    };

    mixxx::Duration difference(const PerformanceTimer& timer) const {
        return mixxx::Duration::fromStdDuration(m_startTime - timer.m_startTime);
    };
    bool running() const {
        return m_startTime != kStoppedTimerValue;
    };

  private:
    std::chrono::time_point<ClockT> m_startTime;
    static constexpr auto kStoppedTimerValue = ClockT::time_point(ClockT::duration::min());
};
