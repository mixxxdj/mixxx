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

    // The standard way of formatting a time in seconds. Used for display of
    // track duration, etc. showCentis indicates whether to include
    // centisecond-precision or to round to the nearest second.
    static QString formatSeconds(double dSeconds, bool showCentis) {
        if (dSeconds < 0) {
            return "?";
        }

        const int days = static_cast<int>(dSeconds) / kSecondsPerDay;
        dSeconds -= days * kSecondsPerDay;

        QTime t = QTime().addMSecs(dSeconds * kMillisPerSecond);

        QString formatString =
                (days > 0 ? (QString::number(days) %
                             QLatin1String("'d', ")) : QString()) %
                QLatin1String(days > 0 || t.hour() > 0 ? "hh:mm:ss" : "mm:ss") %
                QLatin1String(showCentis ? ".zzz" : "");

        QString timeString = t.toString(formatString);

        // The format string gives us milliseconds but we want
        // centiseconds. Slice one character off.
        if (showCentis) {
            timeString = timeString.left(timeString.length() - 1);
        }

        return timeString;
    }

  private:
    static LLTIMER s_timer;

    // For testing timing related behavior.
    static bool s_testMode;
    static qint64 s_testElapsed_nsecs;
};

#endif /* TIME_H */
