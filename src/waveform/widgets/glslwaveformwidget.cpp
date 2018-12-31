#include "glslwaveformwidget.h"

#include <QPainter>
#include <QtDebug>
#include <QOpenGLContext>

#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/renderers/waveformrenderbackground.h"
#include "waveform/renderers/glslwaveformrenderersignal.h"
#include "waveform/renderers/waveformrendererpreroll.h"
#include "waveform/renderers/waveformrendermark.h"
#include "waveform/renderers/waveformrendermarkrange.h"
#include "waveform/renderers/waveformrendererendoftrack.h"
#include "waveform/renderers/waveformrenderbeat.h"

#include "util/performancetimer.h"
#include "util/time.h"

GLSLFilteredWaveformWidget::GLSLFilteredWaveformWidget(const char* group,
                                                       QWidget* parent)
        : GLSLWaveformWidget(group, parent, false) {
}

GLSLRGBWaveformWidget::GLSLRGBWaveformWidget(const char* group, QWidget* parent)
        : GLSLWaveformWidget(group, parent, true) {
}

GLSLWaveformWidget::GLSLWaveformWidget(const char* group, QWidget* parent,
                                       bool rgbRenderer)
        : BaseQOpenGLWidget(group, parent) {
    addRenderer<WaveformRenderBackground>();
    addRenderer<WaveformRendererEndOfTrack>();
    addRenderer<WaveformRendererPreroll>();
    addRenderer<WaveformRenderMarkRange>();
    if (rgbRenderer) {
        signalRenderer_ = addRenderer<GLSLWaveformRendererRGBSignal>();
    } else {
        signalRenderer_ = addRenderer<GLSLWaveformRendererFilteredSignal>();
    }
    addRenderer<WaveformRenderBeat>();
    addRenderer<WaveformRenderMark>();

    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);

    // Initialization requires activating our context.
    if (QOpenGLContext::currentContext() != context()) {
        makeCurrent();
    }
    m_initSuccess = init();
}

GLSLWaveformWidget::~GLSLWaveformWidget() {
    makeCurrent();
}

void GLSLWaveformWidget::castToQWidget() {
    m_widget = static_cast<QWidget*>(static_cast<QOpenGLWidget*>(this));
}

void GLSLWaveformWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
}

mixxx::Duration GLSLWaveformWidget::render() {
    PerformanceTimer timer;
    mixxx::Duration t1;
    //mixxx::Duration t2, t3;
    timer.start();
    // QPainter makes QOpenGLContext::currentContext() == context()
    // this may delayed until previous buffer swap finished
    QPainter painter(this);
    t1 = timer.restart();
    draw(&painter, NULL);
    //t2 = timer.restart();
    //glFinish();
    //t3 = timer.restart();
    //qDebug() << "GLVSyncTestWidget "<< t1 << t2 << t3;
    return t1; // return timer for painter setup
}

void GLSLWaveformWidget::resize(int width, int height, float devicePixelRatio) {
    // NOTE: (vrince) this is needed since we allocation buffer on resize
    // and the Gl Context should be properly set
    makeCurrent();
    WaveformWidgetAbstract::resize(width, height, devicePixelRatio);
}

void GLSLWaveformWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        makeCurrent();
        signalRenderer_->debugClick();
    }
}
