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
/// The c++11 strnlen_s() is not available on all targets
inline std::size_t strnlen(
        const char* str,
        std::size_t maxlen) {
    if (str == nullptr) {
        return 0;
    }
    // Invoke the global function and benefit from SIMD implementations
    return ::strnlen(str, maxlen);
}

/// A nullptr-safe variant of the corresponding standard C function.
///
/// Treats nullptr like an empty string and returns 0.
/// The c++11 wcsnlen_s is not available on all targets
/// and wcsnlen() is not available on OpenBSD
inline std::size_t wcsnlen(
        const wchar_t* wcs,
        std::size_t maxlen) {
    if (wcs == nullptr) {
        return 0;
    }
#if !defined(__BSD__) || defined(__USE_XOPEN2K8)
    // Invoke the global function SIMD implementations
    return ::wcsnlen(wcs, maxlen);
#else
    std::size_t n;
    for (n = 0; n < maxlen; n++) {
        if (!wcs[n]) {
            break;
        }
    }
    return n;
#endif
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
    case sizeof(char16_t):
        return QString::fromUtf16(reinterpret_cast<const char16_t*>(wcs), ilen);
    case sizeof(char32_t):
        return QString::fromUcs4(reinterpret_cast<const char32_t*>(wcs), ilen);
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
