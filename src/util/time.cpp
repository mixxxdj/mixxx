#include "util/time.h"

// static
LLTIMER Time::s_timer;
// static
bool Time::s_testMode = false;
// static
qint64 Time::s_testElapsed_nsecs = 0;

// static
QString Time::formatSeconds(double dSeconds, bool showCentis) {
    if (dSeconds < 0) {
        return "?";
    }

    const int days = static_cast<int>(dSeconds) / kSecondsPerDay;
    dSeconds -= days * kSecondsPerDay;

    // NOTE(uklotzde): Time() constructs a 'null' object, but
    // we need 'zero' here.
    QTime t = QTime(0, 0).addMSecs(dSeconds * kMillisPerSecond);

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
