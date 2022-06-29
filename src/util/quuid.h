#pragma once

#include <QString>
#include <QUuid>

///
/// Utility functions for QUuid
///

/// Format a UUID without enclosing curly braces. Returns a null string
/// instead of 00000000-0000-0000-0000-000000000000.
inline QString uuidToNullableStringWithoutBraces(const QUuid& uuid) {
    if (uuid.isNull()) {
        return {};
    }
    return uuid.toString(QUuid::WithoutBraces);
}

/// Format a UUID without enclosing curly braces and no '-' separators
/// between fields (id128). Returns an empty byte array instead of
/// 00000000000000000000000000000000.
inline QByteArray uuidToCompactAsciiHexDigits(const QUuid& uuid) {
    if (uuid.isNull()) {
        return {};
    }
    return uuid.toByteArray(QUuid::Id128);
}
