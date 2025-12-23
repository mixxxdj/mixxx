#pragma once

#include <QtGlobal> // for qt version checking

// QRegularExpression::escape was introduced in Qt 5.15.
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
#include <QRegularExpression>
#else
#include <QRegExp>
#endif
#include <QStringList>
#include <QString>

namespace RegexUtils {

inline QString fileExtensionsRegex(QStringList extensions) {
    // Escape every extension appropriately
    for (int i = 0; i < extensions.size(); ++i) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        extensions[i] = QRegularExpression::escape(extensions[i]);
#else
        extensions[i] = QRegExp::escape(extensions[i]);
#endif
    }
    // Turn the list into a "\\.(jpg|gif|etc)$" style regex string
    return QStringLiteral("\\.(%1)$").arg(extensions.join('|'));
}

} // namespace RegexUtils
