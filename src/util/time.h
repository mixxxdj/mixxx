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
        return s_timer.elapsed();
    }

    static uint elapsedMsecs() {
        return (uint)(s_timer.elapsed() / 1000);
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
};

#endif /* TIME_H */
