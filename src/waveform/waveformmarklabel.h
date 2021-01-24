#pragma once

#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QRectF>

// WaveformMarkLabel renders the label for a WaveformMark to an offscreen buffer
// and calculates its area. This allows the areas of all WaveformMarkLabels
// to be compared so overlapping labels are not drawn.
class WaveformMarkLabel {
  public:
    WaveformMarkLabel() {};

    // Render the label to an internal QPixmap buffer
    void prerender(QPointF bottomLeft,
            const QPixmap& icon,
            QString text,
            const QFont& font,
            QColor textColor,
            QColor backgroundColor,
            float widgetWidth,
            double scaleFactor);

    // Draw the prerendered pixmap
    void draw(QPainter* pPainter);

    QRectF area() const {
        return m_areaRect;
    };

    void setAreaRect(const QRectF& areaRect) {
        m_areaRect = areaRect;
    }

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
