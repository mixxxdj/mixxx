#pragma once

/// Common utility functions and helpers that are needed for
/// all Qt versions.

#include <QString>

namespace mixxx {

/// Allow to emit non-const signals from a const member function.
///
/// https://github.com/KDE/clazy/blob/master/docs/checks/README-const-signal-or-slot.md
///
/// This is needed as a workaround for many DAO classes. Most member
/// functions should be const and those classes should not emit any
/// signals.
///
/// Usage: emit thisAsNonConst(this)->signalName(<signal args>);
template<typename T>
inline T* thisAsNonConst(const T* constThisPointer) {
    return const_cast<T*>(constThisPointer);
}

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
