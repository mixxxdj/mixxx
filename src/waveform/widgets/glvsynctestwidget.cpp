#include "waveform/widgets/glvsynctestwidget.h"

#include <QPainter>

#include "moc_glvsynctestwidget.cpp"
#include "util/performancetimer.h"
#include "waveform/renderers/glvsynctestrenderer.h"
#include "waveform/renderers/glwaveformrenderbackground.h"

GLVSyncTestWidget::GLVSyncTestWidget(const QString& group, QWidget* parent)
        : GLWaveformWidgetAbstract(group, parent) {
    addRenderer<GLWaveformRenderBackground>(); // 172 µs
    //  addRenderer<WaveformRendererEndOfTrack>(); // 677 µs 1145 µs (active)
    //  addRenderer<WaveformRendererPreroll>(); // 652 µs 2034 µs (active)
    //  addRenderer<WaveformRenderMarkRange>(); // 793 µs

#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
    addRenderer<GLVSyncTestRenderer>(); // 841 µs // 2271 µs
#endif                                  // !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2

    // addRenderer<WaveformRenderMark>(); // 711 µs
    // addRenderer<WaveformRenderBeat>(); // 1183 µs

    m_initSuccess = init();
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
    QPainter painter(paintDevice());
    t1 = timer.restart();
    draw(&painter, nullptr);
    //t2 = timer.restart();
    //qDebug() << "GLVSyncTestWidget "<< t1 << t2;
    return t1; // return timer for painter setup
}
