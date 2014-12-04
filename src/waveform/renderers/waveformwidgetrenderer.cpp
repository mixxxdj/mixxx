#include <QPainter>

#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveform.h"
#include "widget/wwidget.h"
#include "controlobject.h"
#include "controlobjectthread.h"
#include "visualplayposition.h"
#include "util/math.h"
#include "util/performancetimer.h"

const int WaveformWidgetRenderer::s_waveformMinZoom = 1;
const int WaveformWidgetRenderer::s_waveformMaxZoom = 6;

WaveformWidgetRenderer::WaveformWidgetRenderer(const char* group)
    : m_group(group),
      m_height(-1),
      m_width(-1),

      m_firstDisplayedPosition(0.0),
      m_lastDisplayedPosition(0.0),
      m_trackPixelCount(0.0),

      m_zoomFactor(1.0),
      m_rateAdjust(0.0),
      m_visualSamplePerPixel(1.0),
      m_audioSamplePerPixel(1.0),

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
      m_trackSamples(0.0) {

    //qDebug() << "WaveformWidgetRenderer";

#ifdef WAVEFORMWIDGETRENDERER_DEBUG
    m_timer = new QTime();
    currentFrame = 0;
    m_lastFrameTime = 0;
    m_lastSystemFrameTime = 0;
    for (int i = 0; i < 100; ++i) {
        m_lastSystemFramesTime[i] = 0;
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

    //qDebug() << "WaveformWidgetRenderer::init";
    m_visualPlayPosition = VisualPlayPosition::getVisualPlayPosition(m_group);

    m_pRateControlObject = new ControlObjectThread(
            m_group, "rate");
    m_pRateRangeControlObject = new ControlObjectThread(
            m_group, "rateRange");
    m_pRateDirControlObject = new ControlObjectThread(
            m_group, "rate_dir");
    m_pGainControlObject = new ControlObjectThread(
            m_group, "total_gain");
    m_pTrackSamplesControlObject = new ControlObjectThread(
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

    //rate adjust may have change sampling per
    //vRince for the moment only more than one sample per pixel is supported
    //due to the fact we play the visual play pos modulo floor m_visualSamplePerPixel ...
    double visualSamplePerPixel = m_zoomFactor * (1.0 + m_rateAdjust);
    m_visualSamplePerPixel = math_max(1.0, visualSamplePerPixel);

    TrackPointer pTrack(m_pTrack);
    ConstWaveformPointer pWaveform = pTrack ? pTrack->getWaveform() : ConstWaveformPointer();
    int waveformDataSize = pWaveform ? pWaveform->getDataSize() : 0;
    if (pWaveform) {
        m_audioSamplePerPixel = m_visualSamplePerPixel * pWaveform->getAudioVisualRatio();
    } else {
        m_audioSamplePerPixel = 0.0;
    }


    m_playPos = m_visualPlayPosition->getAtNextVSync(vsyncThread);
    // m_playPos = -1 happens, when a new track is in buffer but m_visualPlayPosition was not updated

    if (m_audioSamplePerPixel && m_playPos != -1) {
        // Track length in pixels.
        m_trackPixelCount = static_cast<double>(m_trackSamples) / 2.0 / m_audioSamplePerPixel;

        // Ratio of half the width of the renderer to the track length in
        // pixels. Percent of the track shown in half the waveform widget.
        double displayedLengthHalf = static_cast<double>(m_width) / m_trackPixelCount / 2.0;
        // Avoid pixel jitter in play position by rounding to the nearest track
        // pixel.
        m_playPos = round(m_playPos * m_trackPixelCount) / m_trackPixelCount; // Avoid pixel jitter in play position
        m_playPosVSample = m_playPos * waveformDataSize;

        m_firstDisplayedPosition = m_playPos - displayedLengthHalf;
        m_lastDisplayedPosition = m_playPos + displayedLengthHalf;
    } else {
        m_playPos = -1; // disable renderers
    }

    /*
    qDebug() << "m_group" << m_group
             << "m_trackSamples" << m_trackSamples
             << "m_playPos" << m_playPos
             << "m_rate" << m_rate
             << "m_rateDir" << m_rateDir
             << "m_rateRange" << m_rateRange
             << "m_gain" << m_gain;
             */
}

void WaveformWidgetRenderer::draw(QPainter* painter, QPaintEvent* event) {

#ifdef WAVEFORMWIDGETRENDERER_DEBUG
    m_lastSystemFrameTime = m_timer->restart();
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
            // qDebug() << i << " a  " << timer.restart();
            m_rendererStack.at(i)->draw(painter, event);
            // qDebug() << i << " e " << timer.restart();
        }

        painter->setPen(m_colors.getPlayPosColor());
        painter->drawLine(m_width/2,0,m_width/2,m_height);
        painter->setOpacity(0.5);
        painter->setPen(m_colors.getBgColor());
        painter->drawLine(m_width/2 + 1,0,m_width/2 + 1,m_height);
        painter->drawLine(m_width/2 - 1,0,m_width/2 - 1,m_height);
    }

#ifdef WAVEFORMWIDGETRENDERER_DEBUG
    int systemMax = -1;
    int frameMax = -1;
    for (int i = 0; i < 100; ++i) {
        frameMax = math_max(frameMax, m_lastFramesTime[i]);
        systemMax = math_max(systemMax, m_lastSystemFramesTime[i]);
    }

    //hud debug display
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

    m_lastFrameTime = m_timer->restart();

    ++currentFrame;
    currentFrame = currentFrame%100;
    m_lastSystemFramesTime[currentFrame] = m_lastSystemFrameTime;
    m_lastFramesTime[currentFrame] = m_lastFrameTime;
#endif

    //qDebug() << "draw() ende" << timer.restart();
}

void WaveformWidgetRenderer::resize(int width, int height) {
    m_width = width;
    m_height = height;
    for (int i = 0; i < m_rendererStack.size(); ++i) {
        m_rendererStack[i]->setDirty(true);
        m_rendererStack[i]->onResize();
    }
}

void WaveformWidgetRenderer::setup(const QDomNode& node, const SkinContext& context) {
    m_colors.setup(node, context);
    for (int i = 0; i < m_rendererStack.size(); ++i) {
        m_rendererStack[i]->setup(node, context);
    }
}

void WaveformWidgetRenderer::setZoom(int zoom) {
    //qDebug() << "WaveformWidgetRenderer::setZoom" << zoom;
    m_zoomFactor = math_clamp<double>(zoom, s_waveformMinZoom, s_waveformMaxZoom);
}

void WaveformWidgetRenderer::setTrack(TrackPointer track) {
    m_pTrack = track;
    //used to postpone first display until track sample is actually available
    m_trackSamples = -1.0;

    for (int i = 0; i < m_rendererStack.size(); ++i) {
        m_rendererStack[i]->onSetTrack();
    }
}
