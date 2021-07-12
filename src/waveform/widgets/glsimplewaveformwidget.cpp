#include "glsimplewaveformwidget.h"

#include <QPainter>
#include <QtDebug>

#include "moc_glsimplewaveformwidget.cpp"
#include "util/performancetimer.h"
#include "waveform/renderers/glwaveformrenderersimplesignal.h"
#include "waveform/renderers/waveformrenderbackground.h"
#include "waveform/renderers/waveformrenderbeat.h"
#include "waveform/renderers/waveformrendererendoftrack.h"
#include "waveform/renderers/waveformrendererpreroll.h"
#include "waveform/renderers/waveformrendermark.h"
#include "waveform/renderers/waveformrendermarkrange.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/sharedglcontext.h"

GLSimpleWaveformWidget::GLSimpleWaveformWidget(const QString& group, QWidget* parent)
        : GLWaveformWidgetAbstract(group, parent) {
    qDebug() << "Created QGLWidget. Context"
             << "Valid:" << context()->isValid()
             << "Sharing:" << context()->isSharing();

    addRenderer<WaveformRenderBackground>();
    addRenderer<WaveformRendererEndOfTrack>();
    addRenderer<WaveformRendererPreroll>();
    addRenderer<WaveformRenderMarkRange>();
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
    m_pGlRenderer = addRenderer<GLWaveformRendererSimpleSignal>();
#endif // !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
    addRenderer<WaveformRenderBeat>();
    addRenderer<WaveformRenderMark>();

    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);

    setAutoBufferSwap(false);

    m_initSuccess = init();
}

GLSimpleWaveformWidget::~GLSimpleWaveformWidget() {
}

void GLSimpleWaveformWidget::castToQWidget() {
    m_widget = this;
}

void GLSimpleWaveformWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
}

mixxx::Duration GLSimpleWaveformWidget::render() {
    PerformanceTimer timer;
    mixxx::Duration t1;
    //mixxx::Duration t2, t3;
    timer.start();
    // QPainter makes QGLContext::currentContext() == context()
    // this may delayed until previous buffer swap finished
    QPainter painter(this);
    t1 = timer.restart();
    draw(&painter, nullptr);
    //t2 = timer.restart();
    //qDebug() << "GLSimpleWaveformWidget" << t1 << t2;
    return t1; // return timer for painter setup
}
