#include "glsimplewaveformwidget.h"

#include <QPainter>

#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/renderers/waveformrenderbackground.h"
#include "waveform/renderers/glwaveformrenderersimplesignal.h"
#include "waveform/renderers/waveformrendererpreroll.h"
#include "waveform/renderers/waveformrendermark.h"
#include "waveform/renderers/waveformrendermarkrange.h"
#include "waveform/renderers/waveformrenderbeat.h"

GLSimpleWaveformWidget::GLSimpleWaveformWidget( const char* group, QWidget* parent) :
    WaveformWidgetAbstract(group),
    QGLWidget(parent) {

    addRenderer<WaveformRenderBackground>();
    addRenderer<WaveformRendererPreroll>();
    addRenderer<WaveformRenderMarkRange>();
    addRenderer<GLWaveformRendererSimpleSignal>();
    addRenderer<WaveformRenderMark>();
    addRenderer<WaveformRenderBeat>();

    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);

    setAutoBufferSwap(false);

    if (QGLContext::currentContext() != context()) {
        makeCurrent();
    }
    init();
}

GLSimpleWaveformWidget::~GLSimpleWaveformWidget(){
    if (QGLContext::currentContext() != context()) {
        makeCurrent();
    }
}

void GLSimpleWaveformWidget::castToQWidget() {
    m_widget = static_cast<QWidget*>(static_cast<QGLWidget*>(this));
}

//here rate adjust is discarded in the simple version to have an tinegral
//number of visual sample per pixel
void GLSimpleWaveformWidget::updateVisualSamplingPerPixel() {
    m_visualSamplePerPixel = m_zoomFactor;
}

void GLSimpleWaveformWidget::paintEvent( QPaintEvent* event) {
    if (QGLContext::currentContext() != context()) {
        makeCurrent();
    }
    QPainter painter(this);
    draw(&painter,event);
}

void GLSimpleWaveformWidget::postRender() {
    QGLWidget::swapBuffers();
}
