#include "waveformwidgetrenderer.h"
#include "waveform/waveform.h"

#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "defs.h"

#include <QPainter>

const int WaveformWidgetRenderer::s_waveformMinZoom = 1;
const int WaveformWidgetRenderer::s_waveformMaxZoom = 6;

WaveformWidgetRenderer::WaveformWidgetRenderer() {
    m_playPosControlObject = NULL;
    m_rateControlObject = NULL;
    m_rateRangeControlObject = NULL;
    m_rateDirControlObject = NULL;
    m_gainControlObject = NULL;
    m_trackSamplesControlObject = NULL;

#ifdef WAVEFORMWIDGETRENDERER_DEBUG
    m_timer = NULL;
#endif
}

WaveformWidgetRenderer::WaveformWidgetRenderer( const char* group) :
    m_group(group),
    m_trackInfoObject(0),
    m_height(-1),
    m_width(-1) {
    //qDebug() << "WaveformWidgetRenderer";

    m_firstDisplayedPosition = 0.0;
    m_lastDisplayedPosition = 0.0;
    m_rendererTransformationOffset = 0.0;
    m_rendererTransformationGain = 0.0;

    m_zoomFactor = 1.0;
    m_rateAdjust = 0.0;
    m_visualSamplePerPixel = 1.0;
    m_audioSamplePerPixel = 1.0;

    // Really create some to manage those
    m_playPosControlObject = NULL;
    m_playPos = 0.0;
    m_rateControlObject = NULL;
    m_rate = 0.0;
    m_rateRangeControlObject = NULL;
    m_rateRange = 0.0;
    m_rateDirControlObject = NULL;
    m_rateDir = 0.0;
    m_gainControlObject = NULL;
    m_gain = 1.0;
    m_trackSamplesControlObject = NULL;
    m_trackSamples = -1.0;


#ifdef WAVEFORMWIDGETRENDERER_DEBUG
    m_timer = new QTime();
    currentFrame = 0;
    m_lastFrameTime = 0;
    m_lastSystemFrameTime = 0;
    for( int i = 0; i < 100; ++i) {
        m_lastSystemFramesTime[i] = 0;
        m_lastSystemFramesTime[i] = 0;
    }
#endif
}

WaveformWidgetRenderer::~WaveformWidgetRenderer() {
    //qDebug() << "~WaveformWidgetRenderer";

    for( int i = 0; i < m_rendererStack.size(); ++i)
        delete m_rendererStack[i];

    delete m_playPosControlObject;
    delete m_rateControlObject;
    delete m_rateRangeControlObject;
    delete m_rateDirControlObject;
    delete m_gainControlObject;
    delete m_trackSamplesControlObject;

#ifdef WAVEFORMWIDGETRENDERER_DEBUG
    delete m_timer;
#endif
}

void WaveformWidgetRenderer::init() {

    //qDebug() << "WaveformWidgetRenderer::init";

    m_playPosControlObject = new ControlObjectThreadMain(
                ControlObject::getControl( ConfigKey(m_group,"visual_playposition")));
    m_rateControlObject = new ControlObjectThreadMain(
                ControlObject::getControl( ConfigKey(m_group,"rate")));
    m_rateRangeControlObject = new ControlObjectThreadMain(
                ControlObject::getControl( ConfigKey(m_group,"rateRange")));
    m_rateDirControlObject = new ControlObjectThreadMain(
                ControlObject::getControl( ConfigKey(m_group,"rate_dir")));
    m_gainControlObject = new ControlObjectThreadMain(
                ControlObject::getControl( ConfigKey(m_group,"total_gain")));
    m_trackSamplesControlObject = new ControlObjectThreadMain(
                ControlObject::getControl( ConfigKey(m_group,"track_samples")));

    for( int i = 0; i < m_rendererStack.size(); ++i)
        m_rendererStack[i]->init();
}

void WaveformWidgetRenderer::onPreRender() {

    m_trackSamples = m_trackSamplesControlObject->get();
    if( m_trackSamples < 0.0)
        return;

    //Fetch parameters before rendering in order the display all sub-renderers with the same values
    m_playPos = m_playPosControlObject->get();
    m_rate = m_rateControlObject->get();
    m_rateDir = m_rateDirControlObject->get();
    m_rateRange = m_rateRangeControlObject->get();
    m_gain = m_gainControlObject->get();

    //Legacy stuff (Ryan it that OK?) -> Limit our rate adjustment to < 99%, "Bad Things" might happen otherwise.
    m_rateAdjust = m_rateDir * math_min(0.99, m_rate * m_rateRange);

    //rate adjst may have change sampling per
    updateVisualSamplingPerPixel();

    if (m_trackInfoObject) {
        updateAudioSamplingPerPixel();
        double displayedLength = 2.0*(double)m_width * getAudioSamplePerPixel() / ((double)m_trackSamples);
        m_firstDisplayedPosition = m_playPos - displayedLength / 2.0;
        m_lastDisplayedPosition = m_playPos + displayedLength / 2.0;
        m_rendererTransformationOffset = - m_firstDisplayedPosition;
        m_rendererTransformationGain = m_width / (m_lastDisplayedPosition - m_firstDisplayedPosition);
    }
    else {
        m_firstDisplayedPosition = 0.0;
        m_lastDisplayedPosition = 0.0;
        m_rendererTransformationOffset = 0.0;
        m_rendererTransformationGain = 0.0;
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

void WaveformWidgetRenderer::draw( QPainter* painter, QPaintEvent* event) {

#ifdef WAVEFORMWIDGETRENDERER_DEBUG
    m_lastSystemFrameTime = m_timer->elapsed();
    m_timer->restart();
#endif

    //not ready to display need to wait until track initialization is done
    //draw only first is stack (backgroung)
    if( m_trackSamples < 0.0) {
        if( !m_rendererStack.empty())
            m_rendererStack[0]->draw( painter, event);
        return;
    } else {

        for( int i = 0; i < m_rendererStack.size(); ++i)
            m_rendererStack[i]->draw( painter, event);

        painter->setPen(QColor(255,255,255,200));
        painter->drawLine( m_width/2, 0, m_width/2, m_height);
    }

#ifdef WAVEFORMWIDGETRENDERER_DEBUG
    int systemMax = -1;
    int frameMax = -1;
    for( int i = 0; i < 100; ++i) {
        frameMax = math_max( frameMax, m_lastFramesTime[i]);
        systemMax = math_max( systemMax, m_lastSystemFramesTime[i]);
    }

    //hud debug display
    painter->drawText(1,12,
                      QString::number(m_lastFrameTime).rightJustified(2,'0') + "(" +
                      QString::number(frameMax).rightJustified(2,'0') + ")" +
                      QString::number(m_lastSystemFrameTime) + "(" +
                      QString::number(systemMax) + ")");

    painter->drawText(1,m_height-1,
                      QString::number(m_playPos) + " [" +
                      QString::number(m_firstDisplayedPosition) + "-" +
                      QString::number(m_lastDisplayedPosition) + "]" +
                      QString::number(m_rate) + " | " +
                      QString::number(m_gain) + " | " +
                      QString::number(m_rateDir) + " | " +
                      QString::number(m_zoomFactor));

    m_lastFrameTime = m_timer->elapsed();
    m_timer->restart();

    ++currentFrame;
    currentFrame = currentFrame%100;
    m_lastSystemFramesTime[currentFrame] = m_lastSystemFrameTime;
    m_lastFramesTime[currentFrame] = m_lastFrameTime;
#endif

}

void WaveformWidgetRenderer::resize( int width, int height) {
    m_width = width;
    m_height = height;
    for( int i = 0; i < m_rendererStack.size(); ++i) {
        m_rendererStack[i]->setDirty(true);
        m_rendererStack[i]->onResize();
    }
}

void WaveformWidgetRenderer::setup( const QDomNode& node) {
    for( int i = 0; i < m_rendererStack.size(); ++i)
        m_rendererStack[i]->setup(node);
}

void WaveformWidgetRenderer::setZoom(int zoom) {
    //qDebug() << "WaveformWidgetRenderer::setZoom" << zoom;
    m_zoomFactor = zoom;
    m_zoomFactor = math_max( s_waveformMinZoom, math_min( m_zoomFactor, s_waveformMaxZoom));
    updateVisualSamplingPerPixel();
    updateAudioSamplingPerPixel();
}

//vRince for the moment only more than one sample per pixel is supported
//due to the fact we play the visual play pos modulo floor m_visualSamplePerPixel ...
void WaveformWidgetRenderer::updateVisualSamplingPerPixel() {
    m_visualSamplePerPixel = m_zoomFactor * (1.0 + m_rateAdjust);
    m_visualSamplePerPixel = math_max( 1.0, m_visualSamplePerPixel);
}

void WaveformWidgetRenderer::updateAudioSamplingPerPixel() {
    if( !m_trackInfoObject) {
        m_audioSamplePerPixel = 0.0;
        return;
    }
    m_audioSamplePerPixel = getVisualSamplePerPixel()*m_trackInfoObject->getWaveform()->getAudioVisualRatio();
}

double WaveformWidgetRenderer::getVisualSamplePerPixel() const {
    return m_visualSamplePerPixel;
}

double WaveformWidgetRenderer::getAudioSamplePerPixel() const {
    return m_audioSamplePerPixel;
}

void WaveformWidgetRenderer::regulateVisualSample( int& sampleIndex) const {
    if( m_visualSamplePerPixel < 1.0)
        return;

    sampleIndex -= sampleIndex%(2*int(m_visualSamplePerPixel));
}

void WaveformWidgetRenderer::regulateAudioSample(int& sampleIndex) const
{
    if( m_audioSamplePerPixel < 1.0)
        return;

    sampleIndex -= sampleIndex%(2*(int)m_audioSamplePerPixel);
}

double WaveformWidgetRenderer::transformSampleIndexInRendererWorld( int sampleIndex) const
{
    const double relativePosition = (double)sampleIndex / (double)m_trackSamples;
    return transformPositionInRendererWorld(relativePosition);
}

double WaveformWidgetRenderer::transformPositionInRendererWorld( double position) const
{
    return m_rendererTransformationGain * ( position + m_rendererTransformationOffset);
}

void WaveformWidgetRenderer::setTrack(TrackPointer track)
{
    m_trackInfoObject = track;
    m_playPos = 0.0;
    //used to postpone first display until track sample is actually available
    m_trackSamples = -1.0;

    for( int i = 0; i < m_rendererStack.size(); ++i) {
        m_rendererStack[i]->onSetTrack();
    }
}


