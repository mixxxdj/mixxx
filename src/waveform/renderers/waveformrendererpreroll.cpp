#include <QBrush>
#include <QPen>
#include <QPainter>
#include <QPolygonF>

#include "waveform/renderers/waveformrendererpreroll.h"

#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveform.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"
#include "util/painterscope.h"

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

    double firstDisplayedPosition = m_waveformRenderer->getFirstDisplayedPosition();
    double lastDisplayedPosition = m_waveformRenderer->getLastDisplayedPosition();

    // Check if the pre- or post-roll is on screen. If so, draw little triangles
    // to indicate the respective zones.
    bool preRollVisible = firstDisplayedPosition < 0;
    bool postRollVisible = lastDisplayedPosition > 1;
    if (preRollVisible || postRollVisible) {
        double playMarkerPosition = m_waveformRenderer->getPlayMarkerPosition();
        double samplesPerPixel = m_waveformRenderer->getVisualSamplePerPixel();
        double numberOfSamples = m_waveformRenderer->getLength() * samplesPerPixel;

        int currentPosition = m_waveformRenderer->getPlayPosVSample();
        int totalSamples = m_waveformRenderer->getTotalVSample();
        //qDebug() << "currentPosition" << currentPosition
        //         << "lastDisplayedPosition" << lastDisplayedPosition
        //         << "samplesPerPixel" << samplesPerPixel
        //         << "numberOfSamples" << numberOfSamples
        //         << "totalSamples" << totalSamples
        //         << "WaveformRendererPreroll::playMarkerPosition=" << playMarkerPosition;

        const int polyLength = static_cast<int>(40.0 / samplesPerPixel);
        const float halfBreadth = m_waveformRenderer->getBreadth() / 2.0;
        const float halfPolyBreadth = m_waveformRenderer->getBreadth() / 5.0;

        PainterScope PainterScope(painter);

        painter->setRenderHint(QPainter::Antialiasing);
        //painter->setRenderHint(QPainter::HighQualityAntialiasing);
        //painter->setBackgroundMode(Qt::TransparentMode);
        painter->setWorldMatrixEnabled(false);
        painter->setPen(QPen(QBrush(m_color), std::max(1.0, scaleFactor())));

        // Rotate if drawing vertical waveforms
        if (m_waveformRenderer->getOrientation() == Qt::Vertical) {
            painter->setTransform(QTransform(0, 1, 1, 0, 0, 0));
        }

        if (preRollVisible) {
            // Sample position of the right-most triangle's tip
            int index = static_cast<int>(numberOfSamples * playMarkerPosition - currentPosition);

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
        }

        if (postRollVisible) {
            int remainingVSamples = totalSamples - currentPosition;
            // Sample position of the left-most triangle's tip
            int index = (playMarkerPosition * numberOfSamples) + remainingVSamples;
            qreal endPos = index / samplesPerPixel;
            //painter->drawLine(endPos, 0, endPos, m_waveformRenderer->getBreadth());

            QPolygonF polygon;
            polygon << QPointF(0, halfBreadth)
                    << QPointF(polyLength, halfBreadth - halfPolyBreadth)
                    << QPointF(polyLength, halfBreadth + halfPolyBreadth);

            polygon.translate(endPos, 0);
            while (index < numberOfSamples) {
                painter->drawPolygon(polygon);
                polygon.translate(+(polyLength + 1), 0);
                index += (polyLength + 1) * samplesPerPixel;
            }
        }
    }
}
