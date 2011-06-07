#include "emptywaveform.h"

#include <QPainter>

#include "waveformwidgetrenderer.h"
#include "waveformrenderbackground.h"

EmptyWaveform::EmptyWaveform( const char* group, QWidget* parent) :
    QWidget(parent),
    WaveformWidgetAbstract(group)
{
    //Empty means just a background ;)
    m_waveformWidgetRenderer->addRenderer<WaveformRenderBackground>();
    m_waveformWidgetRenderer->init();
}

EmptyWaveform::~EmptyWaveform()
{
}

void EmptyWaveform::castToQWidget()
{
    m_widget = dynamic_cast<QWidget*>(this);
}

void EmptyWaveform::paintEvent( QPaintEvent* event)
{
    QPainter painter(this);
    m_waveformWidgetRenderer->draw(&painter,event);
}
