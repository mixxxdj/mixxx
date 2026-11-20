#include "track/taglib/fmpsplaycount.h"

#include <QRegularExpression>
#include <limits>

namespace mixxx {

namespace taglib {

std::optional<QString> formatFmpsPlayCount(int playCount) {
    if (playCount < 0) {
        return std::nullopt;
    }
    return QString::number(playCount);
}

std::optional<int> parseFmpsPlayCount(const QString& fmpsPlayCount) {
    // Validate on the string level and convert only the integer part.
    // This avoids QString::toDouble(), which accepts "nan" and "inf"
    // that cannot be rejected afterwards when compiling with -ffast-math.
    static const QRegularExpression kFmpsPlayCountFormat(
            QStringLiteral("^(\\d+)(\\.\\d+)?$"));
    const auto match = kFmpsPlayCountFormat.match(fmpsPlayCount.trimmed());
    if (!match.hasMatch()) {
        return std::nullopt;
    }
    bool ok = false;
    const qulonglong value = match.captured(1).toULongLong(&ok);
    if (!ok || value > static_cast<qulonglong>(std::numeric_limits<int>::max())) {
        // Only digits were matched, so a failed conversion is an overflow
        return std::numeric_limits<int>::max();
    }
    return static_cast<int>(value);
}

} // namespace taglib

} // namespace mixxx
