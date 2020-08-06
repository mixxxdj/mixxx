#include "waveformrenderplaymarker.h"

WaveformRenderPlayMarker::WaveformRenderPlayMarker(
        WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRendererAbstract(waveformWidgetRenderer) {
}

WaveformRenderPlayMarker::~WaveformRenderPlayMarker() {
}

void WaveformRenderPlayMarker::setup(const QDomNode& node, const SkinContext& context) {
    Q_UNUSED(node);
    Q_UNUSED(context);
}

void WaveformRenderPlayMarker::draw(QPainter* painter, QPaintEvent* event) {
    Q_UNUSED(event);
    auto orientation = m_waveformRenderer->getOrientation();
    const int height = m_waveformRenderer->getHeight();
    const int width = m_waveformRenderer->getWidth();
    const double playMarkerPosition = m_waveformRenderer->getPlayMarkerPosition();
    const int lineX = width * playMarkerPosition;
    const int lineY = height * playMarkerPosition;
    const auto colors = m_waveformRenderer->getWaveformSignalColors();
    painter->setPen(Qt::white);
    painter->setPen(colors->getPlayPosColor());
    if (orientation == Qt::Horizontal) {
        painter->drawLine(lineX, 0, lineX, height);
    } else {
        painter->drawLine(0, lineY, width, lineY);
    }
    painter->setOpacity(0.5);
    painter->setPen(colors->getBgColor());
    if (orientation == Qt::Horizontal) {
        painter->drawLine(lineX + 1, 0, lineX + 1, height);
        painter->drawLine(lineX - 1, 0, lineX - 1, height);
    } else {
        painter->drawLine(0, lineY + 1, width, lineY + 1);
        painter->drawLine(0, lineY - 1, width, lineY - 1);
    }
}
