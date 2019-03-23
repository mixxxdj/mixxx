#include "util/duration.h"

#include <QtGlobal>
#include <QStringBuilder>
#include <QTime>

#include "util/assert.h"
#include "util/math.h"

namespace mixxx {

namespace {

static const qint64 kSecondsPerMinute = 60;
static const qint64 kSecondsPerHour = 60 * kSecondsPerMinute;
static const qint64 kSecondsPerDay = 24 * kSecondsPerHour;

} // namespace

// static
const QString DurationBase::kInvalidDurationString = "?";
// Unicode for thin space
QChar DurationBase::kKiloGroupSeparator = QChar(0x2009);
// Unicode for decimal point
QChar DurationBase::kDecimalSeparator = QChar(0x002E);

// static
QString DurationBase::formatTime(double dSeconds, Precision precision) {
    if (dSeconds < 0.0) {
        // negative durations are not supported
        return kInvalidDurationString;
    }

    const qint64 days = static_cast<qint64>(std::floor(dSeconds)) / kSecondsPerDay;
    dSeconds -= days * kSecondsPerDay;

    // NOTE(uklotzde): QTime() constructs a 'null' object, but
    // we need 'zero' here.
    QTime t = QTime(0, 0).addMSecs(dSeconds * kMillisPerSecond);

    QString formatString =
            QLatin1String(t.hour() > 0 && days < 1 ? "hh:mm:ss" : "mm:ss") %
            QLatin1String(Precision::SECONDS == precision ? "" : ".zzz");

    QString durationString = t.toString(formatString);
    if (days > 0) {
        durationString = QString("%1:").arg(days * 24 + t.hour()) % durationString;
    }

    // The format string gives us milliseconds but we want
    // centiseconds. Slice one character off.
    if (Precision::CENTISECONDS == precision) {
        DEBUG_ASSERT(1 <= durationString.length());
        durationString = durationString.left(durationString.length() - 1);
    }

    return durationString;
}

// static
QString DurationBase::formatSeconds(double dSeconds, Precision precision) {
    if (dSeconds < 0.0) {
        // negative durations are not supported
        return kInvalidDurationString;
    }

    QString durationString;

    if (Precision::CENTISECONDS == precision) {
        durationString = QString::number(dSeconds, 'f', 2);
    } else if (Precision::MILLISECONDS == precision) {
        durationString = QString::number(dSeconds, 'f', 3);
    } else {
        durationString = QString::number(dSeconds, 'f', 0);
    }

    return durationString;
}

// static
QString DurationBase::formatSecondsLong(double dSeconds, Precision precision) {
    if (dSeconds < 0.0) {
        // negative durations are not supported
        return kInvalidDurationString;
    }

    QString durationString;

    if (Precision::CENTISECONDS == precision) {
        durationString = QString::number(dSeconds, 'f', 2)
            .rightJustified(6, QLatin1Char('0'));
    } else if (Precision::MILLISECONDS == precision) {
        durationString = QString::number(dSeconds, 'f', 3)
            .rightJustified(7, QLatin1Char('0'));
    } else {
        durationString = QString::number(dSeconds, 'f', 0)
            .rightJustified(3, QLatin1Char('0'));
    }

    return durationString;
}

// static
QString DurationBase::formatKiloSeconds(double dSeconds, Precision precision) {
    if (dSeconds < 0.0) {
        // negative durations are not supported
        return kInvalidDurationString;
    }

    int kilos = (int)dSeconds / 1000;
    double seconds = floor(fmod(dSeconds, 1000));
    double subs = fmod(dSeconds, 1);

    QString durationString =
            QString("%1%2%3").arg(
                    QString::number(kilos),
                    QString(kDecimalSeparator),
                    QString::number(seconds).rightJustified(3, QLatin1Char('0')));
    if (Precision::SECONDS != precision) {
            durationString += kKiloGroupSeparator % QString::number(subs, 'f', 3).right(3);
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
