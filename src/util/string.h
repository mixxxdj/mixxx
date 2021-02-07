#pragma once

#include <QCollator>
#include <QColor>
#include <QLocale>
#include <QString>
#include <QStringRef>
#include <cstring>
#include <cwchar>

#include "util/assert.h"

namespace mixxx {

// The default comparison of strings for sorting.
class StringCollator {
  public:
    explicit StringCollator(QLocale locale = QLocale())
            : m_collator(std::move(locale)) {
        m_collator.setCaseSensitivity(Qt::CaseInsensitive);
    }

    int compare(const QString& s1, const QString& s2) const {
        return m_collator.compare(s1, s2);
    }

    int compare(const QStringRef& s1, const QStringRef& s2) const {
        return m_collator.compare(s1, s2);
    }

  private:
    QCollator m_collator;
};

/// A nullptr-safe variant of the corresponding standard C function.
///
/// Treats nullptr like an empty string and returns 0.
inline std::size_t strnlen(
        const char* str,
        std::size_t len) {
    if (str == nullptr) {
        return 0;
    }
    // Invoke the global function
    return ::strnlen(str, len);
}

/// A nullptr-safe variant of the corresponding standard C function.
///
/// Treats nullptr like an empty string and returns 0.
inline std::size_t wcsnlen(
        const wchar_t* wcs,
        std::size_t len) {
    if (wcs == nullptr) {
        return 0;
    }
    // Invoke the global function
    return ::wcsnlen(wcs, len);
}

/// Convert a wide-character C string to QString.
///
/// We cannot use Qts wchar_t functions, since they may work or not
/// depending on the '/Zc:wchar_t-' build flag in the Qt configs
/// on Windows build.
///
/// See also: QString::fromWCharArray()
inline QString convertWCStringToQString(
        const wchar_t* wcs,
        std::size_t len) {
    if (!wcs) {
        DEBUG_ASSERT(len == 0);
        return QString();
    }
    DEBUG_ASSERT(wcsnlen(wcs, len) == len);
    const auto ilen = static_cast<int>(len);
    DEBUG_ASSERT(ilen >= 0); // unsigned -> signed
    switch (sizeof(wchar_t)) {
    case sizeof(ushort):
        return QString::fromUtf16(reinterpret_cast<const ushort*>(wcs), ilen);
    case sizeof(uint):
        return QString::fromUcs4(reinterpret_cast<const uint*>(wcs), ilen);
    default:
        DEBUG_ASSERT(!"unsupported character type");
        return QString();
    }
}

} // namespace mixxx

// Helper to create html link strings to be used for ui files, mostly in
// Preferences dialogs.
inline QString coloredLinkString(
        const QColor& color,
        const QString& text,
        const QString& baseUrl,
        const QString& extUrl = nullptr) {
    return QStringLiteral("<a style=\"color:") + color.name() +
            QStringLiteral(";\" href=\"") + baseUrl + extUrl +
            QStringLiteral("\">") + text + QStringLiteral("</a>");
}
