#pragma once

#include <QCollator>
#include <QColor>
#include <QLocale>
#include <QString>
#include <QStringRef>

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
