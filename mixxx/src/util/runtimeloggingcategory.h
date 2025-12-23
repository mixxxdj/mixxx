#pragma once

#include <QLoggingCategory>
#include <QString>

/// RuntimeLoggingCategory is a wrapper around QLoggingCategory
/// that facilitates using a QLoggingCategory for a category
/// name string that is determined at run time. To use a
/// RuntimeLoggingCategory, simply pass it to the
/// qCInfo, qCDebug, qCWarning, or qCCritical macros like a
/// QLoggingCategory. If the category name is fully known at
/// compile time, simply construct a QLoggingCategory with a
/// string literal instead of using RuntimeLoggingCategory.
///
/// RuntimeLoggingCategory stores the logging category name as a
/// QByteArray. This is helpful because QLoggingCategory has a deleted
/// copy constructor, does not take ownership nor make a copy of the
/// const char* passed to its constructor, and requires that the
/// data pointed to by the const char* lives as long as the
/// QLoggingCategory.
///
/// RuntimeLoggingCategory takes advantage of QByteArray's
/// implicit sharing to ensure that the const char* for the
/// QLoggingCategory lives as long as the QLoggingCategory without
/// requiring excessive deep copies.
struct RuntimeLoggingCategory {
  public:
    explicit RuntimeLoggingCategory(const QString& categoryName)
            : m_categoryName(categoryName.toLocal8Bit()),
              m_logger(m_categoryName.constData()) {
    }

    explicit RuntimeLoggingCategory(const RuntimeLoggingCategory& other)
            : m_categoryName(other.m_categoryName),
              m_logger(m_categoryName.constData()) {
    }

    /// Implicit converter for passing a RuntimeLoggingCategory to the
    /// qCInfo/qCDebug/qCWarning/qCCritical macros as if it was a
    /// QLoggingCategory.
    operator const QLoggingCategory&() const {
        return m_logger;
    }

    /// Converter for accessing QLoggingCategory members such as the
    /// `isDebugEnabled()` method.
    const QLoggingCategory& operator()() const {
        return m_logger;
    }

    static QString removeInvalidCharsFromCategory(QString categoryName);

  private:
    const QByteArray m_categoryName;
    const QLoggingCategory m_logger;
};
