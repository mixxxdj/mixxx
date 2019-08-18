#include <QPainter>

#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveform.h"
#include "widget/wwidget.h"
#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "waveform/visualplayposition.h"
#include "util/math.h"
#include "util/performancetimer.h"

const double WaveformWidgetRenderer::s_waveformMinZoom = 1.0;
const double WaveformWidgetRenderer::s_waveformMaxZoom = 10.0;
const double WaveformWidgetRenderer::s_waveformDefaultZoom = 3.0;
const double WaveformWidgetRenderer::s_defaultPlayMarkerPosition = 0.5;

WaveformWidgetRenderer::WaveformWidgetRenderer(const char* group)
    : m_group(group),
      m_orientation(Qt::Horizontal),
      m_height(-1),
      m_width(-1),
      m_devicePixelRatio(1.0f),

      m_firstDisplayedPosition(0.0),
      m_lastDisplayedPosition(0.0),
      m_trackPixelCount(0.0),

      m_zoomFactor(1.0),
      m_rateAdjust(0.0),
      m_visualSamplePerPixel(1.0),
      m_audioSamplePerPixel(1.0),
      m_alphaBeatGrid(90),
      // Really create some to manage those;
      m_visualPlayPosition(NULL),
      m_playPos(-1),
      m_playPosVSample(0),
      m_pRateControlObject(NULL),
      m_rate(0.0),
      m_pRateRangeControlObject(NULL),
      m_rateRange(0.0),
      m_pRateDirControlObject(NULL),
      m_rateDir(0.0),
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

    delete m_pRateControlObject;
    delete m_pRateRangeControlObject;
    delete m_pRateDirControlObject;
    delete m_pGainControlObject;
    delete m_pTrackSamplesControlObject;

#ifdef WAVEFORMWIDGETRENDERER_DEBUG
    delete m_timer;
#endif
}

bool WaveformWidgetRenderer::init() {

    //qDebug() << "WaveformWidgetRenderer::init, m_group=" << m_group;

    m_visualPlayPosition = VisualPlayPosition::getVisualPlayPosition(m_group);

    m_pRateControlObject = new ControlProxy(
            m_group, "rate");
    m_pRateRangeControlObject = new ControlProxy(
            m_group, "rateRange");
    m_pRateDirControlObject = new ControlProxy(
            m_group, "rate_dir");
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
    m_trackSamples = m_pTrackSamplesControlObject->get();
    if (m_trackSamples <= 0.0) {
        return;
    }

    //Fetch parameters before rendering in order the display all sub-renderers with the same values
    m_rate = m_pRateControlObject->get();
    m_rateDir = m_pRateDirControlObject->get();
    m_rateRange = m_pRateRangeControlObject->get();

    // This gain adjustment compensates for an arbitrary /2 gain chop in
    // EnginePregain. See the comment there.
    m_gain = m_pGainControlObject->get() * 2;

    //Legacy stuff (Ryan it that OK?) -> Limit our rate adjustment to < 99%, "Bad Things" might happen otherwise.
    m_rateAdjust = m_rateDir * math_min(0.99, m_rate * m_rateRange);

    // Compute visual sample to pixel ratio
    // Allow waveform to spread one visual sample across a hundred pixels
    // NOTE: The hundred pixel limit is totally arbitrary. Theoretically,
    // there should be no limit to how far the waveforms can be zoomed in.
    double visualSamplePerPixel = m_zoomFactor * (1.0 + m_rateAdjust) / m_scaleFactor;
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
        m_playPosVSample = m_playPos * m_trackPixelCount * m_visualSamplePerPixel;

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
    //        "m_rate" << m_rate <<
    //        "m_rateDir" << m_rateDir <<
    //        "m_rateRange" << m_rateRange <<
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

        const int lineX = m_width * m_playMarkerPosition;
        const int lineY = m_height * m_playMarkerPosition;

        painter->setPen(m_colors.getPlayPosColor());
        if (m_orientation == Qt::Horizontal) {
            painter->drawLine(lineX, 0, lineX, m_height);
        } else {
            painter->drawLine(0, lineY, m_width, lineY);
        }
        painter->setOpacity(0.5);
        painter->setPen(m_colors.getBgColor());
        if (m_orientation == Qt::Horizontal) {
            painter->drawLine(lineX + 1, 0, lineX + 1, m_height);
            painter->drawLine(lineX - 1, 0, lineX - 1, m_height);
        } else {
            painter->drawLine(0, lineY + 1, m_width, lineY + 1);
            painter->drawLine(0, lineY - 1, m_width, lineY - 1);
        }
    }

#ifdef WAVEFORMWIDGETRENDERER_DEBUG
    int systemMax = -1;
    int frameMax = -1;
    for (int i = 0; i < 100; ++i) {
        frameMax = math_max(frameMax, m_lastFramesTime[i]);
        systemMax = math_max(systemMax, m_lastSystemFramesTime[i]);
    }

    // hud debug display
    painter->drawText(1,12,
                      QString::number(m_lastFrameTime).rightJustified(2,'0') + "(" +
                      QString::number(frameMax).rightJustified(2,'0') + ")" +
                      QString::number(m_lastSystemFrameTime) + "(" +
                      QString::number(systemMax) + ")" +
                      QString::number(realtimeError));

    painter->drawText(1,m_height-1,
                      QString::number(m_playPos) + " [" +
                      QString::number(m_firstDisplayedPosition) + "-" +
                      QString::number(m_lastDisplayedPosition) + "]" +
                      QString::number(m_rate) + " | " +
                      QString::number(m_gain) + " | " +
                      QString::number(m_rateDir) + " | " +
                      QString::number(m_zoomFactor));

    m_lastFrameTime = m_timer->restart().toIntegerNanos();

    ++currentFrame;
    currentFrame = currentFrame%100;
    m_lastSystemFramesTime[currentFrame] = m_lastSystemFrameTime;
    m_lastFramesTime[currentFrame] = m_lastFrameTime;
#endif

    //qDebug() << "draw() end" << timer.restart().formatNanosWithUnit();
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
