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
/// The c11 strnlen_s() is not available on all targets
inline std::size_t strnlen_s(
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
/// The c11 wcsnlen_s is not available on all targets
/// and wcsnlen() is not available on OpenBSD
inline std::size_t wcsnlen_s(
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
        std::size_t maxLen) {
    if (!wcs) {
        DEBUG_ASSERT(maxLen == 0);
        return QString();
    }

    std::size_t len = wcsnlen_s(wcs, maxLen + 1);
    // Assert when the string wcs has more characters than the specified maximum size
    DEBUG_ASSERT(len <= maxLen);
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
