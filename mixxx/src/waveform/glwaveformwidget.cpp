#include "glwaveformwidget.h"

#include <QPainter>

#include "waveformwidgetrenderer.h"
#include "waveformrenderbackground.h"
#include "glwaveformrendererfilteredsignal.h"
#include "waveformrendermark.h"
#include "waveformrendermarkrange.h"

GLWaveformWidget::GLWaveformWidget( const char* group, QWidget* parent) :
    QGLWidget(parent),
    WaveformWidgetAbstract(group)
{
    m_waveformWidgetRenderer->addRenderer<WaveformRenderBackground>();
    m_waveformWidgetRenderer->addRenderer<WaveformRenderMarkRange>();
    m_waveformWidgetRenderer->addRenderer<GLWaveformRendererFilteredSignal>();
    m_waveformWidgetRenderer->addRenderer<WaveformRenderMark>();
    m_waveformWidgetRenderer->init();
}

GLWaveformWidget::~GLWaveformWidget()
{
}

void GLWaveformWidget::castToQWidget()
{
    //TODO vrince try static_cast -> QGLWigget -> QWidget ...
    m_widget = static_cast<QWidget*>(static_cast<QGLWidget*>(this));
    //m_widget = dynamic_cast<QWidget*>(this);
}

void GLWaveformWidget::paintEvent( QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    m_waveformWidgetRenderer->draw(&painter,event);
}
