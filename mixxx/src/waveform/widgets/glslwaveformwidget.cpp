#include "glslwaveformwidget.h"

#include <QPainter>

#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/renderers/waveformrenderbackground.h"
#include "waveform/renderers/glslwaveformrenderersignal.h"
#include "waveform/renderers/waveformrendererpreroll.h"
#include "waveform/renderers/waveformrendermark.h"
#include "waveform/renderers/waveformrendermarkrange.h"
#include "waveform/renderers/waveformrendererendoftrack.h"
#include "waveform/renderers/waveformrenderbeat.h"

GLSLWaveformWidget::GLSLWaveformWidget( const char* group, QWidget* parent) :
    QGLWidget(parent),
    WaveformWidgetAbstract(group) {

    addRenderer<WaveformRenderBackground>();
    addRenderer<WaveformRendererEndOfTrack>();
    addRenderer<WaveformRendererPreroll>();
    addRenderer<WaveformRenderMarkRange>();
    signalRenderer_ = addRenderer<GLSLWaveformRendererSignal>();
    addRenderer<WaveformRenderMark>();
    addRenderer<WaveformRenderBeat>();

    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);

    setAutoBufferSwap(false);

    makeCurrent();
    init();
}

GLSLWaveformWidget::~GLSLWaveformWidget(){
    makeCurrent();
}

void GLSLWaveformWidget::castToQWidget() {
    m_widget = static_cast<QWidget*>(static_cast<QGLWidget*>(this));
}

void GLSLWaveformWidget::paintEvent( QPaintEvent* event) {
    makeCurrent();
    QPainter painter(this);
    draw(&painter,event);
    QGLWidget::swapBuffers();
}

void GLSLWaveformWidget::resize( int width, int height) {
    //NOTE: (vrince) this is needed since we allocation buffer on resize
    //ans the Gl Context should be properly setted
    makeCurrent();
    WaveformWidgetAbstract::resize(width,height);
}

void GLSLWaveformWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    if( event->button() == Qt::RightButton) {
        makeCurrent();
        signalRenderer_->loadShaders();
    }
}
