#pragma once

#include <QString>
#include <QUuid>

///
/// Utility functions for QUuid
///

/// Format a UUID without enclosing curly braces, representing a null UUID
/// by an empty string instead of 00000000-0000-0000-0000-000000000000.
inline QString uuidToNullableStringWithoutBraces(const QUuid& uuid) {
    if (uuid.isNull()) {
        return QString{};
    } else {
        return uuid.toString(QUuid::WithoutBraces);
    }
}
