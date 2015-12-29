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
  : WaveformRendererAbstract(waveformWidgetRenderer) {
}

WaveformRendererPreroll::~WaveformRendererPreroll() {
}

void WaveformRendererPreroll::setup(const QDomNode& node, const SkinContext& context) {
    m_color.setNamedColor(context.selectString(node, "SignalColor"));
    m_color = WSkinColor::getCorrectColor(m_color);
}

void WaveformRendererPreroll::draw(QPainter* painter, QPaintEvent* event) {
    Q_UNUSED(event);
    const TrackPointer track = m_waveformRenderer->getTrackInfo();
    if (!track) {
        return;
    }
    double samplesPerPixel = m_waveformRenderer->getVisualSamplePerPixel();
    double numberOfSamples = m_waveformRenderer->getWidth() * samplesPerPixel;

    int currentPosition = m_waveformRenderer->getPlayPosVSample();
    //qDebug() << "currentPosition" << currentPosition
    //         << "samplesPerPixel" << samplesPerPixel
    //         << "numberOfSamples" << numberOfSamples;

    // Some of the pre-roll is on screen. Draw little triangles to indicate
    // where the pre-roll is located.
    if (currentPosition < numberOfSamples / 2.0) {
        int index = static_cast<int>(numberOfSamples / 2.0 - currentPosition);
        const int polyWidth = static_cast<int>(40.0 / samplesPerPixel);
        const float halfHeight = m_waveformRenderer->getHeight()/2.0;
        const float halfPolyHeight = m_waveformRenderer->getHeight()/5.0;

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        //painter->setRenderHint(QPainter::HighQualityAntialiasing);
        //painter->setBackgroundMode(Qt::TransparentMode);
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
