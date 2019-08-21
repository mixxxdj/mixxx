#pragma once

#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QRectF>

class WaveformMarkLabel {
  public:
    WaveformMarkLabel() {};

    // Render the pixmap to an internal buffer
    void prerender(QPointF bottomLeft, QPixmap icon, QString text,
            QFont font, QColor textColor, QColor backgroundColor,
            float widgetWidth, double scaleFactor);

    // Draw the prerendered pixmap
    void draw(QPainter* pPainter);

    QRectF area() const {
        return m_areaRect;
    };

    bool intersects(const QRectF& other) const {
        return m_areaRect.intersects(other);
    }

    bool intersects(const WaveformMarkLabel& other) const {
        return intersects(other.area());
    }

    void clear() {
        m_text = QString();
        m_pixmap = QPixmap();
        m_areaRect = QRectF();
    }

  private:
    QPixmap m_icon;
    QString m_text;
    QFont m_font;
    QColor m_textColor;
    QColor m_backgroundColor;

    QPixmap m_pixmap;
    QRectF m_areaRect;
};
