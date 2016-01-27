#ifndef UTIL_TIME_H
#define UTIL_TIME_H

#include <QString>

#include "util/performancetimer.h"
#include "util/threadcputimer.h"
#include "util/timer.h"
#include "util/duration.h"

#define LLTIMER PerformanceTimer
//#define LLTIMER ThreadCpuTimer

class Time {
  public:
    static const int kMillisPerSecond = 1000;
    static const int kSecondsPerMinute = 60;
    static const int kSecondsPerHour = 60 * kSecondsPerMinute;
    static const int kSecondsPerDay = 24 * kSecondsPerHour;

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

    enum class Precision {
        SECONDS,
        CENTISECONDS,
        MILLISECONDS
    };

    // The standard way of formatting a time in seconds. Used for display
    // of track duration, etc.
    static QString formatSeconds(double dSeconds,
                                 Precision precision = Time::Precision::SECONDS);

  private:
    static LLTIMER s_timer;

    // For testing timing related behavior.
    static bool s_testMode;
    static mixxx::Duration s_testElapsed;
};

#endif /* UTIL_TIME_H */
