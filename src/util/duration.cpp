#include "util/duration.h"

#include <QtGlobal>
#include <QStringBuilder>
#include <QTime>
#include <math.h>
#include "util/assert.h"
#include "util/math.h"

namespace mixxx {

namespace {

static const qint64 kSecondsPerMinute = 60;
static const qint64 kSecondsPerHour = 60 * kSecondsPerMinute;
static const qint64 kSecondsPerDay = 24 * kSecondsPerHour;

} // namespace

// static
QString DurationBase::formatSeconds(double dSeconds, Precision precision) {
    if (dSeconds < 0.0) {
        // negative durations are not supported
        return "?";
    }

    const qint64 days = static_cast<qint64>(std::floor(dSeconds)) / kSecondsPerDay;
    dSeconds -= days * kSecondsPerDay;

    // NOTE(uklotzde): QTime() constructs a 'null' object, but
    // we need 'zero' here.
    QTime t = QTime(0, 0).addMSecs(dSeconds * kMillisPerSecond);

    QString formatString =
            (days > 0 ? (QString::number(days) %
                         QLatin1String("'d', ")) : QString()) %
            QLatin1String(days > 0 || t.hour() > 0 ? "hh:mm:ss" : "mm:ss") %
            QLatin1String(Precision::SECONDS == precision ? "" : ".zzz");

    QString durationString = t.toString(formatString);

    // The format string gives us milliseconds but we want
    // centiseconds. Slice one character off.
    if (Precision::CENTISECONDS == precision) {
        DEBUG_ASSERT(1 <= durationString.length());
        durationString = durationString.left(durationString.length() - 1);
    }

    return durationString;
}
// static
QString DurationBase::formatKiloSeconds(double dSeconds, Precision precision) {
    if (dSeconds < 0.0) {
        // negative durations are not supported
        return "?";
    }

    const qint64 days = static_cast<qint64>(std::floor(dSeconds)) / kSecondsPerDay;
    dSeconds -= days * kSecondsPerDay;

    int kilos = (int)dSeconds / 1000;
    double seconds = floor(fmod(dSeconds, 1000));
    double subs = fmod(dSeconds, 1);

    QString durationString =
            (days > 0 ? (QString::number(days) %
                         QLatin1String("d, ")) : QString()) %
            QString("%1.%2").arg(kilos, 0, 10).arg(seconds, 3, 'f', 0, QLatin1Char('0'));
    if (Precision::SECONDS != precision) {
            durationString += QString(":") % QString::number(subs, 'f', 3).right(3);
    }

    // The format string gives us milliseconds but we want
    // centiseconds. Slice one character off.
    if (Precision::CENTISECONDS == precision) {
        DEBUG_ASSERT(1 <= durationString.length());
        durationString = durationString.left(durationString.length() - 1);
    }

    return durationString;
}

} // namespace mixxx
