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

void WaveformRendererPreroll::init() {
}

void WaveformRendererPreroll::setup(const QDomNode& node) {
    m_color.setNamedColor(
        WWidget::selectNodeQString(node, "SignalColor"));
    m_color = WSkinColor::getCorrectColor(m_color);
}

void WaveformRendererPreroll::draw(QPainter* painter, QPaintEvent* event) {
    const TrackPointer track = m_waveformRenderer->getTrackInfo();
    if (!track) {
        return;
    }
    const Waveform* waveform = track->getWaveform();
    int samplesPerPixel = m_waveformRenderer->getVisualSamplePerPixel();
    int numberOfSamples = m_waveformRenderer->getWidth() * samplesPerPixel;

    int currentPosition = 0;

    //TODO (vRince) not really accurate since waveform size une visual reasampling and
    //have two mores samples to hold the complete visual data
    currentPosition = m_waveformRenderer->getPlayPos()*waveform->getDataSize();
    m_waveformRenderer->regulateVisualSample(currentPosition);

    // Some of the pre-roll is on screen. Draw little triangles to indicate
    // where the pre-roll is located.
    if (currentPosition < numberOfSamples) {
        painter->save();
        painter->setWorldMatrixEnabled(false);
        painter->setPen(QPen(QBrush(m_color), 1));
        double start_index = 0;
        int end_index = (numberOfSamples - currentPosition) / 2.0;
        QPolygonF polygon;
        const int polyWidth = 40.0 / samplesPerPixel;
        const float halfHeight = m_waveformRenderer->getHeight()/2.0;
        const float halfPolyHeight = m_waveformRenderer->getHeight()/5.0;
        polygon << QPointF(0, halfHeight)
                << QPointF(-polyWidth, halfHeight - halfPolyHeight)
                << QPointF(-polyWidth, halfHeight + halfPolyHeight);
        polygon.translate(end_index/samplesPerPixel, 0);

        int index = end_index;
        while (index > start_index) {
            painter->drawPolygon(polygon);
            polygon.translate(-polyWidth, 0);
            index -= polyWidth;
        }
        painter->restore();
    }
}

