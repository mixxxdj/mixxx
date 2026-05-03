#pragma once

#include <QDateTime>
#include <QTimeZone>
#include <QVariant>

#include "util/assert.h"

namespace mixxx {

/// Common utility functions for safely converting and consistently
/// displaying date time values.

/// Obtain the local date time from an UTC date time.
inline QDateTime localDateTimeFromUtc(
        const QDateTime& dt) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    return QDateTime(dt.date(), dt.time(), QTimeZone::UTC).toLocalTime();
#else
    return QDateTime(dt.date(), dt.time(), Qt::UTC).toLocalTime();
#endif
}

/// Extract a QDateTime from a QVariant.
inline QDateTime convertVariantToDateTime(
        const QVariant& data) {
    DEBUG_ASSERT(data.canConvert<QDateTime>());
    return data.toDateTime();
}

/// Helper to format a QDate according to a format string.
/// If format is empty, uses the system's default locale short format.
inline QString formatDate(
        const QDate& date,
        const QString& format = QString()) {
    if (!date.isValid()) {
        return QString();
    }
    if (format.isEmpty()) {
        return QLocale().toString(date, QLocale::ShortFormat);
    }
    return date.toString(format);
}

/// Helper to format a QDateTime according to a format string.
/// If format is empty, uses the system's default locale short format.
inline QString formatDateTime(
        const QDateTime& dt,
        const QString& format = QString()) {
    if (!dt.isValid()) {
        return QString();
    }
    if (format.isEmpty()) {
        return QLocale().toString(dt, QLocale::ShortFormat);
    }
    return dt.toString(format);
}

/// Format a QDateTime for display to the user using the
/// application's locale settings or specified preference.
inline QString displayLocalDateTime(
        const QDateTime& dt, const QString& format = QString()) {
    return formatDateTime(dt, format);
}

} // namespace mixxx
