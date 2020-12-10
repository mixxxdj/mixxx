#pragma once

#include <QLocale>
#include <QCollator>
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
/// writes the hexadecimal version of the pointer address in a QString
static inline QString pointerToQString(const void* ptr) {
    return QString("0x%1").arg(reinterpret_cast<quintptr>(ptr),
            QT_POINTER_SIZE * 2,
            16,
            QChar('0'));
};
