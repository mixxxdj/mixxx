#ifndef TIME_H
#define TIME_H

#include <QtGlobal>
#include <QString>
#include <QDateTime>
#include <QTime>
#include <QStringBuilder>

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
    static mixxx::Duration elapsedDuration() {
        if (s_testMode) {
            return mixxx::Duration::fromNanos(s_testElapsed_nsecs);
        }
        return mixxx::Duration::fromNanos(s_timer.elapsed());
    }

    // Returns the time elapsed since Mixxx started up in nanoseconds.
    static qint64 elapsed() {
        if (s_testMode) {
            return s_testElapsed_nsecs;
        }
        return s_timer.elapsed();
    }

    // Returns the time elapsed since Mixxx started up in milliseconds.
    static qint64 elapsedMsecs() {
        if (s_testMode) {
            return s_testElapsed_nsecs / 1000000;
        }
        return (s_timer.elapsed() / 1000000);
    }

    // Enable or disable testing mode. In testing mode we allow tests to set the
    // elapsed time we will return.
    static void setTestMode(bool test) {
        s_testMode = test;
    }

    static void setTestElapsedTime(qint64 elapsed) {
        s_testElapsed_nsecs = elapsed;
    }

    static void setTestElapsedMsecs(qint64 elapsed) {
        s_testElapsed_nsecs = elapsed * 1000000;
    }

    enum class Precision {
        SECONDS,
        CENTISECONDS,
        MILLISECONDS
    };

    // The standard way of formatting a time in seconds. Used for display
    // of track duration, etc.
    static QString formatSeconds(
            double dSeconds,
            Precision precision = Time::Precision::SECONDS);

  private:
    static LLTIMER s_timer;

    // For testing timing related behavior.
    static bool s_testMode;
    static qint64 s_testElapsed_nsecs;
};

#endif /* TIME_H */
