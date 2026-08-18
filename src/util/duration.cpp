#include "util/duration.h"

#include <QTime>
#include <QtGlobal>
#include <cmath>

#include "util/assert.h"
#include "util/fpclassify.h"

namespace mixxx {

namespace {

static constexpr qint64 kSecondsPerMinute = 60;
static constexpr qint64 kSecondsPerHour = 60 * kSecondsPerMinute;
static constexpr qint64 kSecondsPerDay = 24 * kSecondsPerHour;

} // namespace

// static
const QString DurationBase::kInvalidDurationString = "?";
// Unicode for thin space
QChar DurationBase::kKiloGroupSeparator = QChar(0x2009);
// Unicode for decimal point
QChar DurationBase::kDecimalSeparator = QChar(0x002E);

// static
QString DurationBase::formatTime(double dSeconds, Precision precision) {
    if (dSeconds < 0.0 || !util_isfinite(dSeconds)
            // Use >= instead of >: 2^63-1 (qint64) is rounded to 2^63 (double)
            || dSeconds >= static_cast<double>(std::numeric_limits<qint64>::max())) {
        // negative durations and infinity or isNaN values are not supported
        return kInvalidDurationString;
    }

    const qint64 days = static_cast<qint64>(std::floor(dSeconds)) / kSecondsPerDay;
    dSeconds -= days * kSecondsPerDay;

    // NOTE(uklotzde): QTime() constructs a 'null' object, but
    // we need 'zero' here.
    QTime t = QTime(0, 0).addMSecs(static_cast<int>(dSeconds * kMillisPerSecond));

    QString formatString =
            (t.hour() > 0 && days < 1 ? QStringLiteral("hh:mm:ss") : QStringLiteral("mm:ss")) +
            (precision == Precision::SECONDS ? QString() : QStringLiteral(".zzz"));

    QString durationString = t.toString(formatString);
    if (days > 0) {
        durationString = QString::number(days * 24 + t.hour()) + QChar(':') + durationString;
    }
    // remove leading 0
    if (durationString.at(0) == '0' && durationString.at(1) != ':') {
        durationString = durationString.right(durationString.length() - 1);
    }

    // The format string gives us milliseconds but we want
    // centiseconds. Slice one character off.
    if (precision == Precision::CENTISECONDS) {
        DEBUG_ASSERT(durationString.length() >= 1);
        durationString = durationString.left(durationString.length() - 1);
    } else if (precision == Precision::DECISECONDS) {
        DEBUG_ASSERT(durationString.length() >= 2);
        durationString = durationString.left(durationString.length() - 2);
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

    if (precision == Precision::DECISECONDS) {
        durationString = QString::number(dSeconds, 'f', 1);
    } else if (precision == Precision::CENTISECONDS) {
        durationString = QString::number(dSeconds, 'f', 2);
    } else if (precision == Precision::MILLISECONDS) {
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

    if (precision == Precision::DECISECONDS) {
        durationString = QString::number(dSeconds, 'f', 1)
                                 .rightJustified(6, QLatin1Char('0'));
    } else if (precision == Precision::CENTISECONDS) {
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
    double seconds = std::floor(fmod(dSeconds, 1000));
    double subs = fmod(dSeconds, 1);

    QString durationString =
            QString("%1%2%3").arg(
                    QString::number(kilos),
                    QString(kDecimalSeparator),
                    QString::number(seconds).rightJustified(3, QLatin1Char('0')));
    if (precision != Precision::SECONDS) {
        durationString += kKiloGroupSeparator % QString::number(subs, 'f', 3).right(3);
    }

    // The format string gives us milliseconds but we want
    // centiseconds. Slice one character off.
    if (precision == Precision::CENTISECONDS) {
        DEBUG_ASSERT(durationString.length() >= 1);
        durationString = durationString.left(durationString.length() - 1);
    } else if (precision == Precision::DECISECONDS) {
        DEBUG_ASSERT(durationString.length() >= 2);
        durationString = durationString.left(durationString.length() - 2);
    }

    return durationString;
}

} // namespace mixxx
