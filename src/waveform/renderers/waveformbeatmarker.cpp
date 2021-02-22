#include "waveform/renderers/waveformbeatmarker.h"

#include "waveform/renderers/geometryutils.h"

namespace {
constexpr int kTriangleEdgeLength = 9;
const auto labelBackgroundColor = QColor(10, 100, 200, 150);
const int markerGreyBrightness = 200;
const auto markerColor = QColor(markerGreyBrightness, markerGreyBrightness, markerGreyBrightness);
constexpr int fontSize = 9;
} // namespace

WaveformBeatMarker::WaveformBeatMarker()
        : m_orientation(Qt::Horizontal) {
}

void WaveformBeatMarker::draw(QPainter* painter) const {
    painter->setBrush(markerColor);
    painter->setPen((QPen(markerColor, 1)));
    painter->setFont(QFont(painter->font().family(), fontSize));
    const int boundingRectHorizontalPaddingPixels = 2;
    if (m_orientation == Qt::Horizontal) {
        painter->drawLine(QPointF(m_position, 0), QPoint(m_position, m_length));
        painter->drawPolygon(getEquilateralTriangle(
                kTriangleEdgeLength, QPointF(m_position, 0), Direction::DOWN));
        QString labelText;
        float maxWidth = 0;
        for (int i = 0; i < m_textDisplayItems.size(); i++) {
            QString text = m_textDisplayItems.at(i);
            int textWidth =
#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
                    painter->fontMetrics().width(text) +
#else
                    painter->fontMetrics().horizontalAdvance(text) +
#endif
                    2 * boundingRectHorizontalPaddingPixels;
            labelText += text + "\n";
            if (textWidth > maxWidth) {
                maxWidth = textWidth;
            }
        }
        QRectF textBoundingRect(m_position + boundingRectHorizontalPaddingPixels,
                kTriangleEdgeLength / 2 + painter->fontMetrics().height(),
                maxWidth,
                painter->fontMetrics().height() * m_textDisplayItems.size());
        QRectF textBoundingRectPadded(
                textBoundingRect.x() - boundingRectHorizontalPaddingPixels,
                textBoundingRect.y(),
                textBoundingRect.width() +
                        2 * boundingRectHorizontalPaddingPixels,
                textBoundingRect.height());
        painter->setBrush(QBrush(labelBackgroundColor));
        painter->setPen(Qt::transparent);
        painter->drawRect(textBoundingRectPadded);
        painter->setPen(markerColor);
        painter->drawText(textBoundingRect, Qt::TextWordWrap, labelText);
    } else {
        painter->drawLine(QPointF(0, m_position), QPoint(m_length, m_position));
        painter->setBrush(markerColor);
        painter->drawPolygon(getEquilateralTriangle(
                kTriangleEdgeLength, QPointF(0, m_position), Direction::RIGHT));
    }
}
