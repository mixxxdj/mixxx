#pragma once

/// Common utility functions and helpers that are needed for
/// all Qt versions.

#include <QString>

namespace mixxx {

/// Escape special characters in text properties to prevent
/// the implicit creation of shortcuts for widgets.
///
/// The given text is supposed to be displayed literally on
/// the screen and no shortcuts shall be created.
///
/// Needed for: QAction, QAbstractButton
inline QString escapeTextPropertyWithoutShortcuts(QString text) {
    text.replace(QChar('&'), QStringLiteral("&&"));
    return text;
}

} // namespace mixxx
