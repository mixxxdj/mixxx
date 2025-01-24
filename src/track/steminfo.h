#pragma once

#include <QColor>
#include <QObject>
#include <QString>

class StemInfo {
  public:
    StemInfo(const QString& label = QString(), const QColor& color = QColor())
            : m_label(label), m_color(color) {
    }

    const QString& getLabel() const {
        return m_label;
    }
    void setLabel(const QString& label) {
        m_label = label;
    }

    const QColor& getColor() const {
        return m_color;
    }
    void setColor(const QColor& color) {
        m_color = color;
    }

    bool isValid() const {
        return !m_label.isEmpty() && m_color.isValid();
    }

  private:
    QString m_label;
    QColor m_color;
};
Q_DECLARE_METATYPE(StemInfo);

bool operator==(
        const StemInfo& lhs,
        const StemInfo& rhs);

inline bool operator!=(
        const StemInfo& lhs,
        const StemInfo& rhs) {
    return !(lhs == rhs);
}

QDebug operator<<(QDebug debug, const StemInfo& stemInfo);
