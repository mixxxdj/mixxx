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
    m_width(-1),
    m_lastFrameTime(0),
    m_playPosControlObject(0),
    m_playPos(0.0)
{
    m_timer = new QTime();
}

WaveformWidgetRenderer::~WaveformWidgetRenderer()
{
    delete m_timer;
    for( int i = 0; i < m_rendererStack.size(); ++i)
        delete m_rendererStack[i];
}

void WaveformWidgetRenderer::init()
{
    m_playPosControlObject = ControlObject::getControl( ConfigKey(m_group,"visual_playposition"));

    for( int i = 0; i < m_rendererStack.size(); ++i)
        m_rendererStack[i]->init();
}

void WaveformWidgetRenderer::draw( QPainter* painter, QPaintEvent* event)
{
    m_lastSystemFrameTime = m_timer->elapsed();
    m_timer->restart();

    //Fetch parameters before rendering in order the display all sub-renderers with the same values
    if( m_playPosControlObject)
        m_playPos = m_playPosControlObject->get();

    for( int i = 0; i < m_rendererStack.size(); ++i)
        m_rendererStack[i]->draw( painter, event);

    painter->setPen(Qt::white);
    painter->setWorldMatrixEnabled(false);
    painter->drawLine( m_width/2, 0, m_width/2, m_height);
    painter->drawText(1,10,QString::number(m_lastFrameTime) + "/" + QString::number(m_lastSystemFrameTime));

    m_lastFrameTime = m_timer->elapsed();
    m_timer->restart();
}

void WaveformWidgetRenderer::resize( int width, int height)
{
    m_width = width;
    m_height = height;
    for( int i = 0; i < m_rendererStack.size(); ++i)
    {
        m_rendererStack[i]->setDirty(true);
        m_rendererStack[i]->init();
    }
}

void WaveformWidgetRenderer::setup( const QDomNode& node)
{
    for( int i = 0; i < m_rendererStack.size(); ++i)
        m_rendererStack[i]->setup(node);
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


