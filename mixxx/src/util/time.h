#pragma once

#include <chrono>

#include "util/duration.h"
#include "util/performancetimer.h"

namespace mixxx {

class Time {
    using LLTIMER = PerformanceTimer;

  public:
    using rep = LLTIMER::ClockT::rep;
    using duration = LLTIMER::ClockT::duration;
    using period = LLTIMER::ClockT::period;
    using time_point = LLTIMER::ClockT::time_point;

    // default to underlying clock. In testmode, all bets are off anyways.
    static constexpr bool is_steady = LLTIMER::ClockT::is_steady;

    static void start() {
        s_timer.start();
    }
    // <chrono> like interface that includes the testMode hack.
    static time_point now() {
        if (s_testMode) {
            return s_testElapsed;
        }
        return time_point(s_timer.elapsed().toStdDuration());
    }

    // Returns a Duration representing time elapsed since Mixxx started up.
    static mixxx::Duration elapsed() {
        if (s_testMode) {
            return Duration::fromStdDuration(s_testElapsed.time_since_epoch());
        }
        return s_timer.elapsed();
    }

    // Enable or disable testing mode. In testing mode we allow tests to set the
    // elapsed time we will return.
    static void setTestMode(bool test) {
        s_testMode = test;
    }
    template<class Rep, class Period>
    static void addTestTime(std::chrono::duration<Rep, Period> elapsed) {
        s_testElapsed += elapsed;
    }

  private:
    static LLTIMER s_timer;

    // For testing timing related behavior.
    static bool s_testMode;
    static time_point s_testElapsed;
};

// Working around incomplete C++20 support on MacOS
#if defined(__cpp_lib_chrono) && __cpp_lib_chrono >= 201907L
static_assert(std::chrono::is_clock_v<Time>);
#endif

} // namespace mixxx
