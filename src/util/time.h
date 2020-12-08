#pragma once

#include "util/performancetimer.h"
#include "util/threadcputimer.h"
#include "util/duration.h"

namespace mixxx {

#define LLTIMER PerformanceTimer
//#define LLTIMER ThreadCpuTimer

class Time {
  public:
    static void start() {
        s_timer.start();
    }

    // Returns a Duration representing time elapsed since Mixxx started up.
    static mixxx::Duration elapsed() {
        if (s_testMode) {
            return s_testElapsed;
        }
        return s_timer.elapsed();
    }

    // Enable or disable testing mode. In testing mode we allow tests to set the
    // elapsed time we will return.
    static void setTestMode(bool test) {
        s_testMode = test;
    }

    static void setTestElapsedTime(mixxx::Duration elapsed) {
        s_testElapsed = elapsed;
    }

  private:
    static LLTIMER s_timer;

    // For testing timing related behavior.
    static bool s_testMode;
    static mixxx::Duration s_testElapsed;
};

} // namespace mixxx
