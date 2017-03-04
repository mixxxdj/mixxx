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

void WaveformRendererPreroll::setup(
        const QDomNode& node, const SkinContext& context) {
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
    double numberOfSamples = m_waveformRenderer->getLength() * samplesPerPixel;

    int currentPosition = m_waveformRenderer->getPlayPosVSample();
    //qDebug() << "currentPosition" << currentPosition
    //         << "samplesPerPixel" << samplesPerPixel
    //         << "numberOfSamples" << numberOfSamples;

    // Some of the pre-roll is on screen. Draw little triangles to indicate
    // where the pre-roll is located.
    if (currentPosition < numberOfSamples / 2.0) {
        int index = static_cast<int>(numberOfSamples / 2.0 - currentPosition);
        const int polyLength = static_cast<int>(40.0 / samplesPerPixel);
        const float halfBreadth = m_waveformRenderer->getBreadth() / 2.0;
        const float halfPolyBreadth = m_waveformRenderer->getBreadth() / 5.0;

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        //painter->setRenderHint(QPainter::HighQualityAntialiasing);
        //painter->setBackgroundMode(Qt::TransparentMode);
        painter->setWorldMatrixEnabled(false);
        painter->setPen(QPen(QBrush(m_color), std::max(1.0, scaleFactor())));

        // Rotate if drawing vertical waveforms
        if (m_waveformRenderer->getOrientation() == Qt::Vertical) {
            painter->setTransform(QTransform(0, 1, 1, 0, 0, 0));
        }

        QPolygonF polygon;
        polygon << QPointF(0, halfBreadth)
                << QPointF(-polyLength, halfBreadth - halfPolyBreadth)
                << QPointF(-polyLength, halfBreadth + halfPolyBreadth);

        // Draw at most one not or halve visible polygon at the widget borders
        if (index > (numberOfSamples + ((polyLength + 1) * samplesPerPixel))) {
            int rest = index - numberOfSamples;
            rest %= (int)((polyLength + 1) * samplesPerPixel);
            index = numberOfSamples + rest;
        }

        polygon.translate(((qreal)index) / samplesPerPixel, 0);
        while (index > 0) {
            painter->drawPolygon(polygon);
            polygon.translate(-(polyLength + 1), 0);
            index -= (polyLength + 1) * samplesPerPixel;
        }

        painter->restore();
    }
}
