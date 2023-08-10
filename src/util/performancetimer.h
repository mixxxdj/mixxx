#pragma once
#include <chrono>

#include "util/duration.h"

class PerformanceTimer {
  public:
    // TODO: make this configurable via a template parameter?
    // note that std::chrono::steady_clock is not steady on all platforms
    using ClockT = std::chrono::steady_clock;
    constexpr PerformanceTimer()
            : m_startTime(){};

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

    constexpr mixxx::Duration difference(const PerformanceTimer& timer) const {
        return mixxx::Duration::fromStdDuration(m_startTime - timer.m_startTime);
    };

    constexpr bool running() const {
        return m_startTime.time_since_epoch().count() != 0;
    };

  private:
    std::chrono::time_point<ClockT> m_startTime;
};
