#include "waveformwidgetrenderer.h"
#include "waveform/waveform.h"

#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "defs.h"

#include <QPainter>

WaveformWidgetRenderer::WaveformWidgetRenderer() :
    m_timer(0) {
}

WaveformWidgetRenderer::WaveformWidgetRenderer( const char* group) :
    m_group(group),
    m_trackInfoObject(0),
    m_height(-1),
    m_width(-1) {

    m_timer = new QTime();

    m_firstDisplayedPosition = 0.0;
    m_lastDisplayedPosition = 0.0;
    m_rendererTransformationOffset = 0.0;
    m_rendererTransformationGain = 0.0;

    m_zoomFactor = 2.0;
    m_rateAdjust = 0.0;
    m_visualSamplePerPixel = 1.0;
    m_audioSamplePerPixel = 1.0;

    //Really create some to manage those
    m_playPosControlObject = 0;
    m_playPos = 0.0;
    m_rateControlObject = 0;
    m_rate = 0.0;
    m_rateRangeControlObject = 0;
    m_rateRange = 0.0;
    m_rateDirControlObject = 0;
    m_rateDir = 0.0;
    m_gainControlObject = 0;
    m_gain = 1.0;
    m_trackSamplesControlObject = 0;
    m_trackSamples = 0;

    //debug
    currentFrame = 0;
    m_lastFrameTime = 0;
    m_lastSystemFrameTime = 0;
    for( int i = 0; i < 100; ++i) {
        m_lastSystemFramesTime[i] = 0;
        m_lastSystemFramesTime[i] = 0;
    }
}

WaveformWidgetRenderer::~WaveformWidgetRenderer() {
    if(m_timer) delete m_timer;
    for( int i = 0; i < m_rendererStack.size(); ++i)
        delete m_rendererStack[i];
}

void WaveformWidgetRenderer::init() {
    // TODO(rryan): WARNING unsafe use of ControlObject. Must use COThreadMain
    m_playPosControlObject = ControlObject::getControl( ConfigKey(m_group,"visual_playposition"));
    m_rateControlObject = ControlObject::getControl( ConfigKey(m_group,"rate"));
    m_rateRangeControlObject = ControlObject::getControl( ConfigKey(m_group,"rate_dir"));
    m_rateDirControlObject = ControlObject::getControl( ConfigKey(m_group,"rateRange"));
    m_gainControlObject = ControlObject::getControl( ConfigKey(m_group,"total_gain"));
    m_trackSamplesControlObject = ControlObject::getControl( ConfigKey(m_group, "track_samples"));

    for( int i = 0; i < m_rendererStack.size(); ++i)
        m_rendererStack[i]->init();
}

void WaveformWidgetRenderer::preRender() {
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

    if(m_trackInfoObject) {
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
}

void WaveformWidgetRenderer::draw( QPainter* painter, QPaintEvent* event) {
    m_lastSystemFrameTime = m_timer->elapsed();
    m_timer->restart();

    for( int i = 0; i < m_rendererStack.size(); ++i)
        m_rendererStack[i]->draw( painter, event);

    painter->setPen(QColor(255,255,255,200));
    painter->drawLine( m_width/2, 0, m_width/2, m_height);

    int systemMax = -1;
    int frameMax = -1;
    for( int i = 0; i < 100; ++i)
    {
        frameMax = math_max( frameMax, m_lastFramesTime[i]);
        systemMax = math_max( systemMax, m_lastSystemFramesTime[i]);
    }

    //hud debug display
    /*
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
                      QString::number(m_gain));

    m_lastFrameTime = m_timer->elapsed();
    m_timer->restart();

    ++currentFrame;
    currentFrame = currentFrame%100;
    m_lastSystemFramesTime[currentFrame] = m_lastSystemFrameTime;
    m_lastFramesTime[currentFrame] = m_lastFrameTime;
    */
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
    m_zoomFactor = zoom;
    m_zoomFactor = math_max( 1.0, math_min( m_zoomFactor, 4.0));
    updateVisualSamplingPerPixel();
    updateAudioSamplingPerPixel();
}

void WaveformWidgetRenderer::zoomIn() {
    m_zoomFactor -= 1.0;
    m_zoomFactor = math_max( 1.0, m_zoomFactor);
    updateVisualSamplingPerPixel();
    updateAudioSamplingPerPixel();
}

void WaveformWidgetRenderer::zoomOut() {
    m_zoomFactor += 1.0;
    m_zoomFactor = math_min( m_zoomFactor, 4.0); //max 400%
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
    if( track.data() && m_trackSamplesControlObject)
        m_trackSamples = (int)m_trackSamplesControlObject->get();
    else
        m_trackSamples = 0;

    for( int i = 0; i < m_rendererStack.size(); ++i) {
        m_rendererStack[i]->onSetTrack();
    }
}


