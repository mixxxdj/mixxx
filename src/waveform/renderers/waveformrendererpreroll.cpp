#include "waveform/renderers/waveformrendererpreroll.h"

#include <QBrush>
#include <QPainter>
#include <QPen>
#include <QPolygonF>

#include "util/painterscope.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "widget/wskincolor.h"

WaveformRendererPreroll::WaveformRendererPreroll(WaveformWidgetRenderer* waveformWidgetRenderer)
  : WaveformRendererAbstract(waveformWidgetRenderer) {
}

WaveformRendererPreroll::~WaveformRendererPreroll() {
}

void WaveformRendererPreroll::setup(
        const QDomNode& node, const SkinContext& context) {
    m_color = QColor(context.selectString(node, "SignalColor"));
    m_color = WSkinColor::getCorrectColor(m_color);
}

void WaveformRendererPreroll::draw(QPainter* painter, QPaintEvent* event) {
    Q_UNUSED(event);

    const TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    if (!pTrack) {
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
        const double numberOfVisibleVSamples = m_waveformRenderer->getLength() * vSamplesPerPixel;

        const int currentVSamplePosition = m_waveformRenderer->getPlayPosVSample();
        const int totalVSamples = m_waveformRenderer->getTotalVSample();
        // qDebug() << "currentVSamplePosition" << currentVSamplePosition
        //          << "lastDisplayedPosition" << lastDisplayedPosition
        //          << "vSamplesPerPixel" << vSamplesPerPixel
        //          << "numberOfVSamples" << numberOfVSamples
        //          << "totalVSamples" << totalVSamples
        //          << "WaveformRendererPreroll::playMarkerPosition=" << playMarkerPositionFrac;

        const float halfBreadth = m_waveformRenderer->getBreadth() / 2.0f;
        const float halfPolyBreadth = m_waveformRenderer->getBreadth() / 5.0f;
        const bool reverseDirection = m_waveformRenderer->isReverseWaveformDirection();

        PainterScope PainterScope(painter);

        painter->setRenderHint(QPainter::Antialiasing);
        //painter->setRenderHint(QPainter::HighQualityAntialiasing);
        //painter->setBackgroundMode(Qt::TransparentMode);
        painter->setWorldMatrixEnabled(false);
        painter->setPen(QPen(QBrush(m_color), std::max(1.0, scaleFactor())));

        const double polyPixelWidth = 40.0 / vSamplesPerPixel;
        const double polyPixelOffset = polyPixelWidth + painter->pen().widthF();
        const double polyVSampleOffset = polyPixelOffset * vSamplesPerPixel;

        // Rotate if drawing vertical waveforms
        if (m_waveformRenderer->getOrientation() == Qt::Vertical) {
            painter->setTransform(QTransform(0, 1, 1, 0, 0, 0));
        }

        if (preRollVisible) {
            // VSample position of the right-most triangle's tip just before the track start
            double triangleTipVSamplePosition =
                    numberOfVisibleVSamples * playMarkerPositionFrac -
                    currentVSamplePosition;
            // Number of invisible VSamples from the right of the viewport boundary
            // to the track start
            double invisibleVSamples = triangleTipVSamplePosition - numberOfVisibleVSamples;
            // get tip position of the last partial visible triangle to draw only visible triangles
            if (invisibleVSamples > 0) {
                triangleTipVSamplePosition -=
                        floor(invisibleVSamples / polyVSampleOffset) *
                        polyVSampleOffset;
            }

            // In reversed mode, mirror the tip around the play marker and
            // flip both the arrow's point direction and the direction the
            // repeating pattern marches off-screen, so it still points
            // toward and recedes away from the actual track content.
            const double shapeWidth = reverseDirection ? polyPixelWidth : -polyPixelWidth;
            const double stepPixel = reverseDirection ? polyPixelOffset : -polyPixelOffset;
            double tipPixel = triangleTipVSamplePosition / vSamplesPerPixel;
            if (reverseDirection) {
                tipPixel = m_waveformRenderer->reflectPixelPosition(tipPixel);
            }

            QPolygonF polygon;
            polygon << QPointF(0, halfBreadth)
                    << QPointF(shapeWidth, halfBreadth - halfPolyBreadth)
                    << QPointF(shapeWidth, halfBreadth + halfPolyBreadth);
            polygon.translate(tipPixel, 0);

            const double stopPixel = reverseDirection
                    ? m_waveformRenderer->getLength()
                    : 0.0;
            for (; reverseDirection ? (tipPixel < stopPixel) : (tipPixel > stopPixel);
                    tipPixel += stepPixel) {
                painter->drawPolygon(polygon);
                polygon.translate(stepPixel, 0);
            }
        }

        if (postRollVisible) {
            const int remainingVSamples = totalVSamples - currentVSamplePosition;
            // VSample position of the left-most triangle's tip just after the track end
            double triangleTipVSamplePosition =
                    numberOfVisibleVSamples * playMarkerPositionFrac +
                    remainingVSamples;
            // Number of invisible VSamples from the track end to the right of the
            // viewport boundary
            double invisibleVSamples = triangleTipVSamplePosition - numberOfVisibleVSamples;
            // get tip position of the fist partial visible triangle to draw only visible triangle
            if (invisibleVSamples > 0) {
                triangleTipVSamplePosition -=
                        floor(invisibleVSamples / polyVSampleOffset) *
                        polyVSampleOffset;
            }

            // See the pre-roll case above for why these flip in reversed mode.
            const double shapeWidth = reverseDirection ? -polyPixelWidth : polyPixelWidth;
            const double stepPixel = reverseDirection ? -polyPixelOffset : polyPixelOffset;
            double tipPixel = triangleTipVSamplePosition / vSamplesPerPixel;
            if (reverseDirection) {
                tipPixel = m_waveformRenderer->reflectPixelPosition(tipPixel);
            }

            QPolygonF polygon;
            polygon << QPointF(0, halfBreadth)
                    << QPointF(shapeWidth, halfBreadth - halfPolyBreadth)
                    << QPointF(shapeWidth, halfBreadth + halfPolyBreadth);
            polygon.translate(tipPixel, 0);

            const double stopPixel = reverseDirection ? 0.0 : numberOfVisibleVSamples;
            for (; reverseDirection ? (tipPixel > stopPixel) : (tipPixel < stopPixel);
                    tipPixel += stepPixel) {
                painter->drawPolygon(polygon);
                polygon.translate(stepPixel, 0);
            }
        }
    }
}
