#include "glslwaveformwidget.h"

#include <QPainter>
#include <QtDebug>

#include "moc_glslwaveformwidget.cpp"
#include "util/performancetimer.h"
#include "waveform/renderers/glslwaveformrenderersignal.h"
#include "waveform/renderers/waveformrenderbackground.h"
#include "waveform/renderers/waveformrenderbeat.h"
#include "waveform/renderers/waveformrendererendoftrack.h"
#include "waveform/renderers/waveformrendererpreroll.h"
#include "waveform/renderers/waveformrendermark.h"
#include "waveform/renderers/waveformrendermarkrange.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/sharedglcontext.h"

GLSLFilteredWaveformWidget::GLSLFilteredWaveformWidget(
        const QString& group,
        QWidget* parent)
        : GLSLWaveformWidget(group, parent, GLSLWaveformWidget::GlslType::Filtered) {
}

GLSLRGBWaveformWidget::GLSLRGBWaveformWidget(
        const QString& group,
        QWidget* parent)
        : GLSLWaveformWidget(group, parent, GLSLWaveformWidget::GlslType::RGB) {
}

GLSLRGBStackedWaveformWidget::GLSLRGBStackedWaveformWidget(
        const QString& group,
        QWidget* parent)
        : GLSLWaveformWidget(group, parent, GLSLWaveformWidget::GlslType::RGBStacked) {
}

GLSLWaveformWidget::GLSLWaveformWidget(
        const QString& group,
        QWidget* parent,
        GlslType type)
        : GLWaveformWidgetAbstract(group, parent) {
    qDebug() << "Created WGLWidget. Context"
             << "Valid:" << isContextValid()
             << "Sharing:" << isContextSharing();

    makeCurrentIfNeeded();

    addRenderer<WaveformRenderBackground>();
    addRenderer<WaveformRendererEndOfTrack>();
    addRenderer<WaveformRendererPreroll>();
    addRenderer<WaveformRenderMarkRange>();
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
    if (type == GlslType::Filtered) {
        m_pGlRenderer = addRenderer<GLSLWaveformRendererFilteredSignal>();
    } else if (type == GlslType::RGB) {
        m_pGlRenderer = addRenderer<GLSLWaveformRendererRGBSignal>();
    } else if (type == GlslType::RGBStacked) {
        m_pGlRenderer = addRenderer<GLSLWaveformRendererStackedSignal>();
    }
#else
    Q_UNUSED(type);
#endif // QT_NO_OPENGL && !QT_OPENGL_ES_2
    addRenderer<WaveformRenderBeat>();
    addRenderer<WaveformRenderMark>();

    m_initSuccess = init();
}

void GLSLWaveformWidget::castToQWidget() {
    m_widget = this;
}

void GLSLWaveformWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
}

mixxx::Duration GLSLWaveformWidget::render() {
    PerformanceTimer timer;
    mixxx::Duration t1;
    //mixxx::Duration t2, t3;
    timer.start();
    // QPainter makes QGLContext::currentContext() == context()
    // this may delayed until previous buffer swap finished
    QPainter painter(paintDevice());
    t1 = timer.restart();
    draw(&painter, nullptr);
    //t2 = timer.restart();
    //qDebug() << "GLSLWaveformWidget" << t1 << t2;
    return t1; // return timer for painter setup
}

void GLSLWaveformWidget::resize(int width, int height) {
    // NOTE: (vrince) this is needed since we allocation buffer on resize
    // and the Gl Context should be properly set
    makeCurrentIfNeeded();
    WaveformWidgetAbstract::resize(width, height);
}

void GLSLWaveformWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        makeCurrentIfNeeded();
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
        if (m_signalRenderer) {
            m_signalRenderer->debugClick();
        }
#endif // !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
    }
}
