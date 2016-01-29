#include "util/time.h"

#include <QtGlobal>
#include <QStringBuilder>
#include <QTime>

#include "util/assert.h"

// static
LLTIMER Time::s_timer;
// static
bool Time::s_testMode = false;
// static
mixxx::Duration Time::s_testElapsed = mixxx::Duration::fromNanos(0);

// static
QString Time::formatSeconds(double dSeconds, Precision precision) {
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
            QLatin1String(Precision::SECONDS == precision ? "" : ".zzz");

    QString timeString = t.toString(formatString);

    // The format string gives us milliseconds but we want
    // centiseconds. Slice one character off.
    if (Precision::CENTISECONDS == precision) {
        DEBUG_ASSERT(1 <= timeString.length());
        timeString = timeString.left(timeString.length() - 1);
    }

    return timeString;
}
