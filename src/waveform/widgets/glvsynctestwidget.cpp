#include "waveform/widgets/glvsynctestwidget.h"

#include <QPainter>
#include <QtDebug>

#include "moc_glvsynctestwidget.cpp"
#include "util/performancetimer.h"
#include "waveform/renderers/glvsynctestrenderer.h"
#include "waveform/renderers/glwaveformrenderersimplesignal.h"
#include "waveform/renderers/waveformrenderbackground.h"
#include "waveform/renderers/waveformrenderbeat.h"
#include "waveform/renderers/waveformrendererendoftrack.h"
#include "waveform/renderers/waveformrendererpreroll.h"
#include "waveform/renderers/waveformrendermark.h"
#include "waveform/renderers/waveformrendermarkrange.h"
#include "waveform/sharedglcontext.h"

GLVSyncTestWidget::GLVSyncTestWidget(const QString& group, QWidget* parent)
        : GLWaveformWidgetAbstract(group, parent) {
    qDebug() << "Created QGLWidget. Context"
             << "Valid:" << context()->isValid()
             << "Sharing:" << context()->isSharing();

    addRenderer<WaveformRenderBackground>(); // 172 µs
//  addRenderer<WaveformRendererEndOfTrack>(); // 677 µs 1145 µs (active)
//  addRenderer<WaveformRendererPreroll>(); // 652 µs 2034 µs (active)
//  addRenderer<WaveformRenderMarkRange>(); // 793 µs

#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
    addRenderer<GLVSyncTestRenderer>(); // 841 µs // 2271 µs
#endif                                  // !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2

    // addRenderer<WaveformRenderMark>(); // 711 µs
    // addRenderer<WaveformRenderBeat>(); // 1183 µs

    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);

    setAutoBufferSwap(false);

    m_initSuccess = init();
    qDebug() << "GLVSyncTestWidget.isSharing() =" << isSharing();
}

GLVSyncTestWidget::~GLVSyncTestWidget() {
}

void GLVSyncTestWidget::castToQWidget() {
    m_widget = this;
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
    draw(&painter, nullptr);
    //t2 = timer.restart();
    //qDebug() << "GLVSyncTestWidget "<< t1 << t2;
    return t1; // return timer for painter setup
}
