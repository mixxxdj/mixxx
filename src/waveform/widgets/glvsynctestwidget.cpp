#include "glvsynctestwidget.h"

#include <QPainter>
#include <QOpenGLContext>

#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/renderers/waveformrenderbackground.h"
#include "waveform/renderers/glwaveformrenderersimplesignal.h"
#include "waveform/renderers/glvsynctestrenderer.h"
#include "waveform/renderers/waveformrendererpreroll.h"
#include "waveform/renderers/waveformrendermark.h"
#include "waveform/renderers/waveformrendermarkrange.h"
#include "waveform/renderers/waveformrendererendoftrack.h"
#include "waveform/renderers/waveformrenderbeat.h"

#include "util/performancetimer.h"

GLVSyncTestWidget::GLVSyncTestWidget(const char* group, QWidget* parent)
        : BaseQOpenGLWidget(group, parent) {
//    addRenderer<WaveformRenderBackground>(); // 172 µs
//    addRenderer<WaveformRendererEndOfTrack>(); // 677 µs 1145 µs (active)
//    addRenderer<WaveformRendererPreroll>(); // 652 µs 2034 µs (active)
//    addRenderer<WaveformRenderMarkRange>(); // 793 µs
    addRenderer<GLVSyncTestRenderer>(); // 841 µs // 2271 µs
//    addRenderer<WaveformRenderMark>(); // 711 µs
//    addRenderer<WaveformRenderBeat>(); // 1183 µs

    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);

    if (QOpenGLContext::currentContext() != context()) {
        makeCurrent();
    }
    m_initSuccess = init();
}

GLVSyncTestWidget::~GLVSyncTestWidget() {
    if (QOpenGLContext::currentContext() != context()) {
        makeCurrent();
    }
}

void GLVSyncTestWidget::castToQWidget() {
    m_widget = static_cast<QWidget*>(static_cast<QOpenGLWidget*>(this));
}

void GLVSyncTestWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
}

mixxx::Duration GLVSyncTestWidget::render() {
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
    glFinish();
    //t3 = timer.restart();
    //qDebug() << "GLVSyncTestWidget "<< t1 << t2 << t3;
    return t1; // return timer for painter setup
}
