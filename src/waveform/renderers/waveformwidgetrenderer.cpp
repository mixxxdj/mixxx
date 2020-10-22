#include "waveform/renderers/waveformwidgetrenderer.h"

#include <QPainter>
#include <QPainterPath>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "track/track.h"
#include "util/math.h"
#include "util/performancetimer.h"
#include "waveform/visualplayposition.h"
#include "waveform/waveform.h"
#include "widget/wwidget.h"

const double WaveformWidgetRenderer::s_waveformMinZoom = 1.0;
const double WaveformWidgetRenderer::s_waveformMaxZoom = 10.0;
const double WaveformWidgetRenderer::s_waveformDefaultZoom = 3.0;
const double WaveformWidgetRenderer::s_defaultPlayMarkerPosition = 0.5;

WaveformWidgetRenderer::WaveformWidgetRenderer(const QString& group)
        : m_group(group),
          m_orientation(Qt::Horizontal),
          m_height(-1),
          m_width(-1),
          m_devicePixelRatio(1.0f),

          m_firstDisplayedPosition(0.0),
          m_lastDisplayedPosition(0.0),
          m_trackPixelCount(0.0),

          m_zoomFactor(1.0),
          m_visualSamplePerPixel(1.0),
          m_audioSamplePerPixel(1.0),
          m_alphaBeatGrid(90),
          // Really create some to manage those;
          m_visualPlayPosition(NULL),
          m_playPos(-1),
          m_playPosVSample(0),
          m_totalVSamples(0),
          m_pRateRatioCO(NULL),
          m_rateRatio(1.0),
          m_pGainControlObject(NULL),
          m_gain(1.0),
          m_pTrackSamplesControlObject(NULL),
          m_trackSamples(0.0),
          m_scaleFactor(1.0),
          m_playMarkerPosition(s_defaultPlayMarkerPosition) {
    //qDebug() << "WaveformWidgetRenderer";

#ifdef WAVEFORMWIDGETRENDERER_DEBUG
    m_timer = new QTime();
    currentFrame = 0;
    m_lastFrameTime = 0;
    for (int i = 0; i < 100; ++i) {
        m_lastFramesTime[i] = 0;
    }
    m_lastSystemFrameTime = 0;
    for (int i = 0; i < 100; ++i) {
        m_lastSystemFramesTime[i] = 0;
    }
#endif
}

WaveformWidgetRenderer::~WaveformWidgetRenderer() {
    //qDebug() << "~WaveformWidgetRenderer";

    for (int i = 0; i < m_rendererStack.size(); ++i)
        delete m_rendererStack[i];

    delete m_pRateRatioCO;
    delete m_pGainControlObject;
    delete m_pTrackSamplesControlObject;

#ifdef WAVEFORMWIDGETRENDERER_DEBUG
    delete m_timer;
#endif
}

bool WaveformWidgetRenderer::init() {
    //qDebug() << "WaveformWidgetRenderer::init, m_group=" << m_group;

    m_visualPlayPosition = VisualPlayPosition::getVisualPlayPosition(m_group);

    m_pRateRatioCO = new ControlProxy(
            m_group, "rate_ratio");
    m_pGainControlObject = new ControlProxy(
            m_group, "total_gain");
    m_pTrackSamplesControlObject = new ControlProxy(
            m_group, "track_samples");

    for (int i = 0; i < m_rendererStack.size(); ++i) {
        if (!m_rendererStack[i]->init()) {
            return false;
        }
    }
    return true;
}

void WaveformWidgetRenderer::onPreRender(VSyncThread* vsyncThread) {
    // For a valid track to render we need
    m_trackSamples = static_cast<int>(m_pTrackSamplesControlObject->get());
    if (m_trackSamples <= 0) {
        return;
    }

    //Fetch parameters before rendering in order the display all sub-renderers with the same values
    m_rateRatio = m_pRateRatioCO->get();

    // This gain adjustment compensates for an arbitrary /2 gain chop in
    // EnginePregain. See the comment there.
    m_gain = m_pGainControlObject->get() * 2;

    // Compute visual sample to pixel ratio
    // Allow waveform to spread one visual sample across a hundred pixels
    // NOTE: The hundred pixel limit is totally arbitrary. Theoretically,
    // there should be no limit to how far the waveforms can be zoomed in.
    double visualSamplePerPixel = m_zoomFactor * m_rateRatio / m_scaleFactor;
    m_visualSamplePerPixel = math_max(0.01, visualSamplePerPixel);

    TrackPointer pTrack(m_pTrack);
    ConstWaveformPointer pWaveform = pTrack ? pTrack->getWaveform() : ConstWaveformPointer();
    if (pWaveform) {
        m_audioSamplePerPixel = m_visualSamplePerPixel * pWaveform->getAudioVisualRatio();
    } else {
        m_audioSamplePerPixel = 0.0;
    }

    double truePlayPos = m_visualPlayPosition->getAtNextVSync(vsyncThread);
    // m_playPos = -1 happens, when a new track is in buffer but m_visualPlayPosition was not updated

    if (m_audioSamplePerPixel && truePlayPos != -1) {
        // Track length in pixels.
        m_trackPixelCount = static_cast<double>(m_trackSamples) / 2.0 / m_audioSamplePerPixel;

        // Avoid pixel jitter in play position by rounding to the nearest track
        // pixel.
        m_playPos = round(truePlayPos * m_trackPixelCount) / m_trackPixelCount;
        m_totalVSamples = static_cast<int>(m_trackPixelCount * m_visualSamplePerPixel);
        m_playPosVSample = static_cast<int>(m_playPos * m_totalVSamples);

        double leftOffset = m_playMarkerPosition;
        double rightOffset = 1.0 - m_playMarkerPosition;

        double displayedLengthLeft = (static_cast<double>(getLength()) / m_trackPixelCount) * leftOffset;
        double displayedLengthRight = (static_cast<double>(getLength()) / m_trackPixelCount) * rightOffset;

        //qDebug() << "WaveformWidgetRenderer::onPreRender" <<
        //        "m_playMarkerPosition=" << m_playMarkerPosition <<
        //        "leftOffset=" << leftOffset <<
        //        "rightOffset=" << rightOffset <<
        //        "displayedLengthLeft=" << displayedLengthLeft <<
        //        "displayedLengthRight=" << displayedLengthRight;

        m_firstDisplayedPosition = m_playPos - displayedLengthLeft;
        m_lastDisplayedPosition = m_playPos + displayedLengthRight;
    } else {
        m_playPos = -1; // disable renderers
    }

    //qDebug() << "WaveformWidgetRenderer::onPreRender" <<
    //        "m_group" << m_group <<
    //        "m_trackSamples" << m_trackSamples <<
    //        "m_playPos" << m_playPos <<
    //        "m_rateRatio" << m_rate <<
    //        "m_gain" << m_gain;
}

void WaveformWidgetRenderer::draw(QPainter* painter, QPaintEvent* event) {
#ifdef WAVEFORMWIDGETRENDERER_DEBUG
    m_lastSystemFrameTime = m_timer->restart().toIntegerNanos();
#endif

    //PerformanceTimer timer;
    //timer.start();

    // not ready to display need to wait until track initialization is done
    // draw only first is stack (background)
    int stackSize = m_rendererStack.size();
    if (m_trackSamples <= 0.0 || m_playPos == -1) {
        if (stackSize) {
            m_rendererStack.at(0)->draw(painter, event);
        }
        return;
    } else {
        for (int i = 0; i < stackSize; i++) {
            //qDebug() << i << " a  " << timer.restart().formatNanosWithUnit();
            m_rendererStack.at(i)->draw(painter, event);
            //qDebug() << i << " e " << timer.restart().formatNanosWithUnit();
        }

        drawPlayPosmarker(painter);
    }

#ifdef WAVEFORMWIDGETRENDERER_DEBUG
    int systemMax = -1;
    int frameMax = -1;
    for (int i = 0; i < 100; ++i) {
        frameMax = math_max(frameMax, m_lastFramesTime[i]);
        systemMax = math_max(systemMax, m_lastSystemFramesTime[i]);
    }

    // hud debug display
    painter->drawText(1,
            12,
            QString::number(m_lastFrameTime).rightJustified(2, '0') + "(" +
                    QString::number(frameMax).rightJustified(2, '0') + ")" +
                    QString::number(m_lastSystemFrameTime) + "(" +
                    QString::number(systemMax) + ")" +
                    QString::number(realtimeError));

    painter->drawText(1,
            m_height - 1,
            QString::number(m_playPos) + " [" +
                    QString::number(m_firstDisplayedPosition) + "-" +
                    QString::number(m_lastDisplayedPosition) + "]" +
                    QString::number(m_rate) + " | " + QString::number(m_gain) +
                    " | " + QString::number(m_rateDir) + " | " +
                    QString::number(m_zoomFactor));

    m_lastFrameTime = m_timer->restart().toIntegerNanos();

    ++currentFrame;
    currentFrame = currentFrame % 100;
    m_lastSystemFramesTime[currentFrame] = m_lastSystemFrameTime;
    m_lastFramesTime[currentFrame] = m_lastFrameTime;
#endif

    //qDebug() << "draw() end" << timer.restart().formatNanosWithUnit();
}

void WaveformWidgetRenderer::drawPlayPosmarker(QPainter* painter) {
    const int lineX = static_cast<int>(m_width * m_playMarkerPosition);
    const int lineY = static_cast<int>(m_height * m_playMarkerPosition);

    // draw dim outlines to increase playpos/waveform contrast
    painter->setOpacity(0.5);
    painter->setPen(m_colors.getBgColor());
    QBrush bgFill = m_colors.getBgColor();
    if (m_orientation == Qt::Horizontal) {
        // lines next to playpos
        // Note: don't draw lines where they would overlap the triangles,
        // otherwise both translucent strokes add up to a darker tone.
        painter->drawLine(lineX + 1, 4, lineX + 1, m_height);
        painter->drawLine(lineX - 1, 4, lineX - 1, m_height);

        // triangle at top edge
        // Increase line/waveform contrast
        painter->setOpacity(0.8);
        QPointF t0 = QPointF(lineX - 5, 0);
        QPointF t1 = QPointF(lineX + 5, 0);
        QPointF t2 = QPointF(lineX, 6);
        drawTriangle(painter, bgFill, t0, t1, t2);
    } else { // vertical waveforms
        painter->drawLine(4, lineY + 1, m_width, lineY + 1);
        painter->drawLine(4, lineY - 1, m_width, lineY - 1);
        // triangle at left edge
        painter->setOpacity(0.8);
        QPointF l0 = QPointF(0, lineY - 5.01);
        QPointF l1 = QPointF(0, lineY + 4.99);
        QPointF l2 = QPointF(6, lineY);
        drawTriangle(painter, bgFill, l0, l1, l2);
    }

    // draw colored play position indicators
    painter->setOpacity(1.0);
    painter->setPen(m_colors.getPlayPosColor());
    QBrush fgFill = m_colors.getPlayPosColor();
    if (m_orientation == Qt::Horizontal) {
        // play position line
        painter->drawLine(lineX, 0, lineX, m_height);
        // triangle at top edge
        QPointF t0 = QPointF(lineX - 4, 0);
        QPointF t1 = QPointF(lineX + 4, 0);
        QPointF t2 = QPointF(lineX, 5);
        drawTriangle(painter, fgFill, t0, t1, t2);
    } else {
        // vertical waveforms
        painter->drawLine(0, lineY, m_width, lineY);
        // triangle at left edge
        QPointF l0 = QPointF(0, lineY - 4.01);
        QPointF l1 = QPointF(0, lineY + 4);
        QPointF l2 = QPointF(5, lineY);
        drawTriangle(painter, fgFill, l0, l1, l2);
    }
}

void WaveformWidgetRenderer::drawTriangle(QPainter* painter,
        QBrush fillColor,
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

void WaveformWidgetRenderer::resize(int width, int height, float devicePixelRatio) {
    m_width = width;
    m_height = height;
    m_devicePixelRatio = devicePixelRatio;
    for (int i = 0; i < m_rendererStack.size(); ++i) {
        m_rendererStack[i]->setDirty(true);
        m_rendererStack[i]->onResize();
    }
}

void WaveformWidgetRenderer::setup(
        const QDomNode& node, const SkinContext& context) {
    m_scaleFactor = context.getScaleFactor();
    QString orientationString = context.selectString(node, "Orientation").toLower();
    if (orientationString == "vertical") {
        m_orientation = Qt::Vertical;
    } else {
        m_orientation = Qt::Horizontal;
    }

    m_colors.setup(node, context);
    for (int i = 0; i < m_rendererStack.size(); ++i) {
        m_rendererStack[i]->setScaleFactor(m_scaleFactor);
        m_rendererStack[i]->setup(node, context);
    }
}

void WaveformWidgetRenderer::setZoom(double zoom) {
    //qDebug() << "WaveformWidgetRenderer::setZoom" << zoom;
    m_zoomFactor = math_clamp<double>(zoom, s_waveformMinZoom, s_waveformMaxZoom);
}

void WaveformWidgetRenderer::setDisplayBeatGridAlpha(int alpha) {
    m_alphaBeatGrid = alpha;
}

void WaveformWidgetRenderer::setTrack(TrackPointer track) {
    m_pTrack = track;
    //used to postpone first display until track sample is actually available
    m_trackSamples = -1.0;

    for (int i = 0; i < m_rendererStack.size(); ++i) {
        m_rendererStack[i]->onSetTrack();
    }
}

WaveformMarkPointer WaveformWidgetRenderer::getCueMarkAtPoint(QPoint point) const {
    for (const auto& pMark : m_markPositions.keys()) {
        int markImagePositionInWidgetSpace = m_markPositions[pMark];
        QPoint pointInImageSpace;
        if (getOrientation() == Qt::Horizontal) {
            pointInImageSpace = QPoint(point.x() - markImagePositionInWidgetSpace, point.y());
        } else {
            DEBUG_ASSERT(getOrientation() == Qt::Vertical);
            pointInImageSpace = QPoint(point.x(), point.y() - markImagePositionInWidgetSpace);
        }
        if (pMark->contains(pointInImageSpace, getOrientation())) {
            return pMark;
        }
    }
    return nullptr;
}
