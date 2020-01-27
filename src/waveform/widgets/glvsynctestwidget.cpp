#include "glvsynctestwidget.h"

#include <QPainter>

#include "waveform/sharedglcontext.h"
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
    : QGLWidget(parent, SharedGLContext::getWidget()),
      WaveformWidgetAbstract(group) {
    qDebug() << "Created QGLWidget. Context"
             << "Valid:" << context()->isValid()
             << "Sharing:" << context()->isSharing();
    if (QGLContext::currentContext() != context()) {
        makeCurrent();
    }

    addRenderer<WaveformRenderBackground>(); // 172 µs
//    addRenderer<WaveformRendererEndOfTrack>(); // 677 µs 1145 µs (active)
//    addRenderer<WaveformRendererPreroll>(); // 652 µs 2034 µs (active)
//    addRenderer<WaveformRenderMarkRange>(); // 793 µs
    addRenderer<GLVSyncTestRenderer>(); // 841 µs // 2271 µs
//    addRenderer<WaveformRenderMark>(); // 711 µs
//    addRenderer<WaveformRenderBeat>(); // 1183 µs

    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);

    setAutoBufferSwap(false);

    m_initSuccess = init();
    qDebug() << "GLVSyncTestWidget.isSharing() =" << isSharing();
}

GLVSyncTestWidget::~GLVSyncTestWidget() {
    if (QGLContext::currentContext() != context()) {
        makeCurrent();
    }
}

void GLVSyncTestWidget::castToQWidget() {
    m_widget = static_cast<QWidget*>(static_cast<QGLWidget*>(this));
}

void GLVSyncTestWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
}

mixxx::Duration GLVSyncTestWidget::render() {
    PerformanceTimer timer;
    mixxx::Duration t1;
    //mixxx::Duration t2;
    timer.start();
    // QPainter makes QGLContext::currentContext() == context()
    // this may delayed until previous buffer swap finished
    QPainter painter(this);
    t1 = timer.restart();
    draw(&painter, NULL);
    //t2 = timer.restart();
    //qDebug() << "GLVSyncTestWidget "<< t1 << t2;
    return t1; // return timer for painter setup
}
