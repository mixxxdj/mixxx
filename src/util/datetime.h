#pragma once

#include <QDateTime>
#include <QVariant>

#include "util/assert.h"

namespace mixxx {

/// Common utility functions for safely converting and consistently
/// displaying date time values.

/// Obtain the local date time from an UTC date time.
inline QDateTime localDateTimeFromUtc(
        QDateTime dt) {
    dt.setTimeSpec(Qt::UTC);
    return dt.toLocalTime();
}

/// Extract a QDateTime from a QVariant.
inline QDateTime convertVariantToDateTime(
        const QVariant& data) {
    DEBUG_ASSERT(data.canConvert<QDateTime>());
    return data.toDateTime();
}

/// Format a QDateTime for display to the user using the
/// application's locale settings.
inline QString displayLocalDateTime(
        const QDateTime& dt) {
    return QLocale().toString(dt, QLocale::ShortFormat);
}

} // namespace mixxx
