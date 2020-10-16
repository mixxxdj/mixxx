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
        const double playMarkerPositionFrac = m_waveformRenderer->getPlayMarkerPosition();
        const double vSamplesPerPixel = m_waveformRenderer->getVisualSamplePerPixel();
        const double numberOfVSamples = m_waveformRenderer->getLength() * vSamplesPerPixel;

        int currentVSamplePosition = m_waveformRenderer->getPlayPosVSample();
        int totalVSamples = m_waveformRenderer->getTotalVSample();
        // qDebug() << "currentVSamplePosition" << currentVSamplePosition
        //          << "lastDisplayedPosition" << lastDisplayedPosition
        //          << "vSamplesPerPixel" << vSamplesPerPixel
        //          << "numberOfVSamples" << numberOfVSamples
        //          << "totalVSamples" << totalVSamples
        //          << "WaveformRendererPreroll::playMarkerPosition=" << playMarkerPositionFrac;

        const double polyLength = 40.0 / vSamplesPerPixel;
        const double polyVSampleSize = (polyLength + 1) * vSamplesPerPixel;
        const float halfBreadth = m_waveformRenderer->getBreadth() / 2.0f;
        const float halfPolyBreadth = m_waveformRenderer->getBreadth() / 5.0f;

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
            // VSample position of the right-most triangle's tip
            double triangleTipVSamplePosition =
                    numberOfVSamples * playMarkerPositionFrac -
                    currentVSamplePosition;

            QPolygonF polygon;
            polygon << QPointF(0, halfBreadth)
                    << QPointF(-polyLength, halfBreadth - halfPolyBreadth)
                    << QPointF(-polyLength, halfBreadth + halfPolyBreadth);
            polygon.translate(triangleTipVSamplePosition / vSamplesPerPixel, 0);

            for (; triangleTipVSamplePosition > 0; triangleTipVSamplePosition -= polyVSampleSize) {
                painter->drawPolygon(polygon);
                polygon.translate(-(polyLength + 1), 0);
            }
        }

        if (postRollVisible) {
            const int remainingVSamples = totalVSamples - currentVSamplePosition;
            // Sample position of the left-most triangle's tip
            double triangleTipVSamplePosition =
                    playMarkerPositionFrac * numberOfVSamples +
                    remainingVSamples;

            QPolygonF polygon;
            polygon << QPointF(0, halfBreadth)
                    << QPointF(polyLength, halfBreadth - halfPolyBreadth)
                    << QPointF(polyLength, halfBreadth + halfPolyBreadth);
            polygon.translate(triangleTipVSamplePosition / vSamplesPerPixel, 0);

            for (; triangleTipVSamplePosition < numberOfVSamples;
                    triangleTipVSamplePosition += polyVSampleSize) {
                painter->drawPolygon(polygon);
                polygon.translate(polyLength + 1, 0);
            }
        }
    }
}
