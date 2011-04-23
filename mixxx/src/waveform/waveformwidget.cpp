#include "waveformwidget.h"

#include "controlobjectthreadmain.h"
#include "controlobject.h"

#include "waveformrenderbackground.h"

#include <QPainter>

WaveformWidgetRenderer::WaveformWidgetRenderer( const char* group) :
    QObject(),
    WaveformRendererAbstract(this), //redirect to him sefl could be 0 too ...
    m_group(group),
    m_trackInfoObject(0),
    m_height(-1),
    m_width(-1)
{
    m_timer = new QTime();

    m_zoomFactor = 1.0;
    m_rateAdjust = 0.0;

    //Realy create some to manage those
    m_playPosControlObject = 0;
    m_playPos = 0.0;
    m_rateControlObject = 0;
    m_rate = 0.0;
    m_rateRangeControlObject = 0;
    m_rateRange = 0.0;
    m_rateDirControlObject = 0;
    m_rateDir = 0.0;

    //debug
    currentFrame = 0;
    m_lastFrameTime = 0;
    m_lastSystemFrameTime = 0;
    for( int i = 0; i < 100; ++i)
    {
        m_lastSystemFramesTime[i] = 0;
        m_lastSystemFramesTime[i] = 0;
    }
}

WaveformWidgetRenderer::~WaveformWidgetRenderer()
{
    qDebug() << "WaveformWidgetRenderer::WaveformWidgetRenderer()";

    delete m_timer;
    for( int i = 0; i < m_rendererStack.size(); ++i)
        delete m_rendererStack[i];
}

void WaveformWidgetRenderer::init()
{
    qDebug() << "WaveformWidgetRenderer::init()";

    m_playPosControlObject = ControlObject::getControl( ConfigKey(m_group,"visual_playposition"));
    m_rateControlObject = ControlObject::getControl( ConfigKey(m_group,"rate"));
    m_rateRangeControlObject = ControlObject::getControl( ConfigKey(m_group,"rate_dir"));
    m_rateDirControlObject = ControlObject::getControl( ConfigKey(m_group,"rateRange"));

    for( int i = 0; i < m_rendererStack.size(); ++i)
        m_rendererStack[i]->init();
}

void WaveformWidgetRenderer::draw( QPainter* painter, QPaintEvent* event)
{
    m_lastSystemFrameTime = m_timer->elapsed();
    m_timer->restart();

    //Fetch parameters before rendering in order the display all sub-renderers with the same values
    m_playPos = m_playPosControlObject->get();
    m_rate = m_rateControlObject->get();
    m_rateDir = m_rateDirControlObject->get();
    m_rateRange = m_rateRangeControlObject->get();

    //Legacy stuff (Ryan it that OK?)
    //Limit our rate adjustment to < 99%, "Bad Things" might happen otherwise.
    m_rateAdjust = m_rateDir * std::min(0.99, m_rate * m_rateRange);

    for( int i = 0; i < m_rendererStack.size(); ++i)
        m_rendererStack[i]->draw( painter, event);

    painter->setPen(Qt::white);
    painter->setWorldMatrixEnabled(false);
    painter->drawLine( m_width/2, 0, m_width/2, m_height);

    int systemMax = -1;
    int frameMax = -1;
    for( int i = 0; i < 100; ++i)
    {
        frameMax = std::max( frameMax, m_lastFramesTime[i]);
        systemMax = std::max( systemMax, m_lastSystemFramesTime[i]);
    }

    //hud debug display
    painter->drawText(1,12,
                      QString::number(m_lastFrameTime).rightJustified(2,'0') + "(" +
                      QString::number(frameMax).rightJustified(2,'0') + ")" +
                      QString::number(m_lastSystemFrameTime) + "(" +
                      QString::number(systemMax) + ")");

    painter->drawText(1,m_height-1,QString::number(m_playPos) + " | " + QString::number(m_rate));

    m_lastFrameTime = m_timer->elapsed();
    m_timer->restart();

    ++currentFrame;
    currentFrame = currentFrame%100;
    m_lastSystemFramesTime[currentFrame] = m_lastSystemFrameTime;
    m_lastFramesTime[currentFrame] = m_lastFrameTime;
}

void WaveformWidgetRenderer::resize( int width, int height)
{
    qDebug() << "WaveformWidgetRenderer::resize()";

    m_width = width;
    m_height = height;
    for( int i = 0; i < m_rendererStack.size(); ++i)
    {
        m_rendererStack[i]->setDirty(true);
        m_rendererStack[i]->onResize();
    }
}

void WaveformWidgetRenderer::setup( const QDomNode& node)
{
    qDebug() << "WaveformWidgetRenderer::setup()";

    for( int i = 0; i < m_rendererStack.size(); ++i)
        m_rendererStack[i]->setup(node);
}

bool WaveformWidgetRenderer::zoomIn()
{
    if( m_zoomFactor < 1.1) //limit zoom to 100%
        return false;

    m_zoomFactor -= 1.0;
    return true;
}

bool WaveformWidgetRenderer::zoomOut()
{
    if( m_zoomFactor > 4.9) //limit zoom to 500%
        return false;

    m_zoomFactor += 1.0;
    return true;
}

float WaveformWidgetRenderer::getVisualSamplePerPixel()
{
    //vRince for the moment only more than one sample per pixel is supported
    //due to the fact we play the visual play pos modulo floor samplePerPixel ...
    return std::max( 1.0, m_zoomFactor * (1.0 + m_rateAdjust));
}

void WaveformWidgetRenderer::slotNewTrack(TrackPointer track)
{
    m_trackInfoObject = track;
    m_playPos = 0.0;
}

void WaveformWidgetRenderer::slotUnloadTrack( TrackPointer /*track*/)
{
    slotNewTrack( TrackPointer(0));
}


