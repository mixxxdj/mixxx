#include "glwaveformwidget.h"

#include <QPainter>

#include "waveformwidget.h"
#include "waveformrenderbackground.h"
#include "glwaveformrendererfilteredsignal.h"

GLWaveformWidget::GLWaveformWidget( const char* group, QWidget* parent) :
    QGLWidget(parent),
    m_waveformWidgetRenderer( new WaveformWidgetRenderer(group))
{
    m_waveformWidgetRenderer->init();
    m_waveformWidgetRenderer->addRenderer<WaveformRenderBackground>();
    m_waveformWidgetRenderer->addRenderer<GLWaveformRendererFilteredSignal>();
}

GLWaveformWidget::~GLWaveformWidget()
{
    delete m_waveformWidgetRenderer;
}

void GLWaveformWidget::resizeEvent(QResizeEvent* /*event*/)
{
    m_waveformWidgetRenderer->resize( width(), height());
}

void GLWaveformWidget::paintEvent( QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    m_waveformWidgetRenderer->draw(&painter,event);
}
