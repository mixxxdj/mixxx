#include "waveformrenderplaymarker.h"

#include <QPainterPath>

WaveformRenderPlayMarker::WaveformRenderPlayMarker(
        WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRendererAbstract(waveformWidgetRenderer) {
}

void WaveformRenderPlayMarker::setup(
        const QDomNode& node, const SkinContext& context) {
    Q_UNUSED(node);
    Q_UNUSED(context);
}

void WaveformRenderPlayMarker::draw(QPainter* painter, QPaintEvent* event) {
    Q_UNUSED(event);
    auto orientation = m_waveformRenderer->getOrientation();
    const int height = m_waveformRenderer->getHeight();
    const int width = m_waveformRenderer->getWidth();
    const double playMarkerPosition =
            m_waveformRenderer->getPlayMarkerPosition();
    const int lineX = static_cast<int>(width * playMarkerPosition);
    const int lineY = static_cast<int>(height * playMarkerPosition);
    const auto colors = m_waveformRenderer->getWaveformSignalColors();

    // draw dim outlines to increase playpos/waveform contrast
    painter->setOpacity(0.5);
    painter->setPen(colors->getBgColor());
    QBrush bgFill = colors->getBgColor();
    if (orientation == Qt::Horizontal) {
        // lines next to playpos
        // Note: don't draw lines where they would overlap the triangles,
        // otherwise both translucent strokes add up to a darker tone.
        painter->drawLine(lineX + 1, 4, lineX + 1, height);
        painter->drawLine(lineX - 1, 4, lineX - 1, height);

        // triangle at top edge
        // Increase line/waveform contrast
        painter->setOpacity(0.8);
        QPointF t0 = QPointF(lineX - 5, 0);
        QPointF t1 = QPointF(lineX + 5, 0);
        QPointF t2 = QPointF(lineX, 6);
        drawTriangle(painter, bgFill, t0, t1, t2);
    } else { // vertical waveforms
        painter->drawLine(4, lineY + 1, width, lineY + 1);
        painter->drawLine(4, lineY - 1, width, lineY - 1);
        // triangle at left edge
        painter->setOpacity(0.8);
        QPointF l0 = QPointF(0, lineY - 5.01);
        QPointF l1 = QPointF(0, lineY + 4.99);
        QPointF l2 = QPointF(6, lineY);
        drawTriangle(painter, bgFill, l0, l1, l2);
    }

    // draw colored play position indicators
    painter->setOpacity(1.0);
    painter->setPen(colors->getPlayPosColor());
    QBrush fgFill = colors->getPlayPosColor();
    if (orientation == Qt::Horizontal) {
        // play position line
        painter->drawLine(lineX, 0, lineX, height);
        // triangle at top edge
        QPointF t0 = QPointF(lineX - 4, 0);
        QPointF t1 = QPointF(lineX + 4, 0);
        QPointF t2 = QPointF(lineX, 5);
        drawTriangle(painter, fgFill, t0, t1, t2);
    } else {
        // vertical waveforms
        painter->drawLine(0, lineY, width, lineY);
        // triangle at left edge
        QPointF l0 = QPointF(0, lineY - 4.01);
        QPointF l1 = QPointF(0, lineY + 4);
        QPointF l2 = QPointF(5, lineY);
        drawTriangle(painter, fgFill, l0, l1, l2);
    }
}

void WaveformRenderPlayMarker::drawTriangle(QPainter* painter,
        const QBrush& fillColor,
        QPointF p0,
        QPointF p1,
        QPointF p2) {
    QPainterPath triangle;
    painter->setPen(Qt::NoPen);
    triangle.moveTo(p0); // ° base 1
    triangle.lineTo(p1); // > base 2
    triangle.lineTo(p2); // > peak
    triangle.lineTo(p0); // > base 1
    painter->fillPath(triangle, fillColor);
}
