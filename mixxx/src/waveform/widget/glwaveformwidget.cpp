#include "glwaveformwidget.h"

#include <QPainter>

#include "waveform/waveformwidgetrenderer.h"
#include "waveform/waveformrenderbackground.h"
#include "waveform/glwaveformrendererfilteredsignal.h"
#include "waveform/waveformrendermark.h"
#include "waveform/waveformrendermarkrange.h"
#include "waveform/waveformrendererendoftrack.h"
#include "waveform/waveformrenderbeat.h"

GLWaveformWidget::GLWaveformWidget( const char* group, QWidget* parent) :
    QGLWidget(parent),
    WaveformWidgetAbstract(group)
{
    m_waveformWidgetRenderer->addRenderer<WaveformRenderBackground>();
    m_waveformWidgetRenderer->addRenderer<WaveformRendererEndOfTrack>();
    m_waveformWidgetRenderer->addRenderer<WaveformRenderMarkRange>();
    m_waveformWidgetRenderer->addRenderer<WaveformRenderMark>();
    m_waveformWidgetRenderer->addRenderer<GLWaveformRendererFilteredSignal>();
    m_waveformWidgetRenderer->addRenderer<WaveformRenderBeat>();

    m_waveformWidgetRenderer->init();

    setAutoBufferSwap(false);
}

GLWaveformWidget::~GLWaveformWidget()
{
}

void GLWaveformWidget::castToQWidget()
{
    m_widget = static_cast<QWidget*>(static_cast<QGLWidget*>(this));
}

void GLWaveformWidget::paintEvent( QPaintEvent* event)
{
    QPainter painter(this);
    m_waveformWidgetRenderer->draw(&painter,event);
    QGLWidget::swapBuffers();
}
