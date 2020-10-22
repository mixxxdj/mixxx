#include "waveform/waveformmarklabel.h"
#include "util/math.h"

void WaveformMarkLabel::prerender(QPointF bottomLeft, QPixmap icon, QString text,
            QFont font, QColor textColor, QColor backgroundColor,
            float widgetWidth, double scaleFactor) {
    if (text.isEmpty() && icon.isNull()) {
        clear();
        return;
    }

    m_text = text;
    QFontMetrics fontMetrics(font);
    const int padding = 2;

    QRectF pixmapRect;
    pixmapRect = fontMetrics.boundingRect(text);
    float availableWidthForText;
    if (icon.isNull()) {
        pixmapRect.setWidth(padding + pixmapRect.width() + padding);
        availableWidthForText = widgetWidth - padding * 2;
    } else {
        pixmapRect.setWidth(padding + icon.width() + padding + pixmapRect.width() + padding);
        availableWidthForText = widgetWidth - padding * 3;
    }
    // Elide extremely long labels
    if (pixmapRect.width() > widgetWidth) {
        text = fontMetrics.elidedText(
                text, Qt::ElideRight, static_cast<int>(availableWidthForText));
        pixmapRect.setWidth(widgetWidth);
    }
    pixmapRect.setHeight(math_max(fontMetrics.height(), icon.height()));

    // pixmapRect has a top left of (0,0) for rendering to m_pixmap.
    // m_areaRect is the same size but shifted to the coordinates of the widget.
    m_areaRect = pixmapRect;
    QPointF topLeft = QPointF(bottomLeft.x(),
            bottomLeft.y() - pixmapRect.height());
    m_areaRect.moveTo(topLeft);

    if (m_areaRect.right() > widgetWidth) {
        m_areaRect.setLeft(widgetWidth - m_areaRect.width());
    }

    m_pixmap = QPixmap(static_cast<int>(pixmapRect.width() * scaleFactor),
            static_cast<int>(pixmapRect.height() * scaleFactor));
    m_pixmap.setDevicePixelRatio(scaleFactor);
    m_pixmap.fill(Qt::transparent);

    QPainter painter(&m_pixmap);

    painter.setPen(QColor(Qt::transparent));
    painter.setBrush(QBrush(backgroundColor));
    painter.drawRoundedRect(QRectF(0, 0, pixmapRect.width(), pixmapRect.height()), 2.0, 2.0);

    if (!icon.isNull()) {
        QPointF iconTopLeft = pixmapRect.topLeft();
        iconTopLeft.setX(iconTopLeft.x() + padding);
        painter.drawPixmap(iconTopLeft, icon);
    }

    if (!text.isEmpty()) {
        QPointF textBottomLeft;
        textBottomLeft.setX(icon.width() + padding);
        textBottomLeft.setY(fontMetrics.ascent());
        painter.setFont(font);
        painter.setPen(textColor);
        painter.drawText(textBottomLeft, text);
    }
};

void WaveformMarkLabel::draw(QPainter* pPainter) {
    pPainter->drawPixmap(m_areaRect.topLeft(), m_pixmap);
}
