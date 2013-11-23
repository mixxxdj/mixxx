#include <QBrush>
#include <QPen>
#include <QPainter>
#include <QPolygonF>

#include "waveform/renderers/waveformrendererpreroll.h"

#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveform.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

WaveformRendererPreroll::WaveformRendererPreroll(WaveformWidgetRenderer* waveformWidgetRenderer)
  : WaveformRendererAbstract( waveformWidgetRenderer) {
}

WaveformRendererPreroll::~WaveformRendererPreroll() {
}

void WaveformRendererPreroll::setup(const QDomNode& node) {
    m_color.setNamedColor(
        WWidget::selectNodeQString(node, "SignalColor"));
    m_color = WSkinColor::getCorrectColor(m_color);
}

void WaveformRendererPreroll::draw(QPainter* painter, QPaintEvent* event) {
    Q_UNUSED(event);
    const TrackPointer track = m_waveformRenderer->getTrackInfo();
    if (!track) {
        return;
    }
    const Waveform* waveform = track->getWaveform();
    double samplesPerPixel = m_waveformRenderer->getVisualSamplePerPixel();
    double numberOfSamples = m_waveformRenderer->getWidth() * samplesPerPixel;

    // TODO (vRince) not really accurate since waveform size une visual reasampling and
    // have two mores samples to hold the complete visual data
    int currentPosition = m_waveformRenderer->getPlayPos() * waveform->getDataSize();
    m_waveformRenderer->regulateVisualSample(currentPosition);

    // Some of the pre-roll is on screen. Draw little triangles to indicate
    // where the pre-roll is located.
    if (currentPosition < numberOfSamples) {
        int index = static_cast<int>((numberOfSamples - currentPosition) / 2.0);
        const int polyWidth = static_cast<int>(40.0 / samplesPerPixel);
        const float halfHeight = m_waveformRenderer->getHeight()/2.0;
        const float halfPolyHeight = m_waveformRenderer->getHeight()/5.0;

        painter->save();
        painter->setWorldMatrixEnabled(false);
        painter->setPen(QPen(QBrush(m_color), 1));
        QPolygonF polygon;
        polygon << QPointF(0, halfHeight)
                << QPointF(-polyWidth, halfHeight - halfPolyHeight)
                << QPointF(-polyWidth, halfHeight + halfPolyHeight);

        // Draw at most one not or halve visible polygon at the widget borders
        if (index > (numberOfSamples + ((polyWidth + 1) * samplesPerPixel))) {
            int rest = index - numberOfSamples;
            rest %= (int)((polyWidth + 1) * samplesPerPixel);
            index = numberOfSamples + rest;
        }

        polygon.translate(((qreal)index) / samplesPerPixel, 0);
        while (index > 0) {
            painter->drawPolygon(polygon);
            polygon.translate(-(polyWidth + 1), 0);
            index -= (polyWidth + 1) * samplesPerPixel;
        }
        painter->restore();
    }
}

