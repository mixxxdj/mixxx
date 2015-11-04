#ifndef TIME_H
#define TIME_H

#include <QtGlobal>
#include <QString>
#include <QTime>

#include "util/performancetimer.h"
#include "util/threadcputimer.h"
#include "util/timer.h"

#define LLTIMER PerformanceTimer
//#define LLTIMER ThreadCpuTimer

class Time {
  public:
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
    // track duration, etc. showMillis indicates whether to include
    // millisecond-precision or to round to the nearest second.
    static QString formatSeconds(int seconds, bool showMillis) {
        if (seconds < 0)
            return "?";
        QTime t = QTime().addSecs(seconds);
        QString formatString = (t.hour() >= 1) ? "hh:mm:ss" : "mm:ss";
        if (showMillis)
            formatString = formatString.append(".zzz");
        QString timeString = t.toString(formatString);
        // The format string gives us one extra digit of millisecond precision than
        // we care about. Slice it off.
        if (showMillis)
            timeString = timeString.left(timeString.length() - 1);
        return timeString;
    }

  private:
    static LLTIMER s_timer;
};

#endif /* TIME_H */
