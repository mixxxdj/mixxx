#include "waveform/widgets/qtsimplewaveformwidget.h"

#include <QPainter>
#include <QtDebug>

#include "moc_qtsimplewaveformwidget.cpp"
#include "util/performancetimer.h"
#include "waveform/renderers/qtwaveformrenderersimplesignal.h"
#include "waveform/renderers/waveformrenderbackground.h"
#include "waveform/renderers/waveformrenderbeat.h"
#include "waveform/renderers/waveformrendererendoftrack.h"
#include "waveform/renderers/waveformrendererpreroll.h"
#include "waveform/renderers/waveformrendermark.h"
#include "waveform/renderers/waveformrendermarkrange.h"
#include "waveform/renderers/waveformwidgetrenderer.h"

QtSimpleWaveformWidget::QtSimpleWaveformWidget(
        const QString& group,
        QWidget* parent)
        : GLWaveformWidgetAbstract(group, parent) {
    qDebug() << "Created WGLWidget. Context"
             << "Valid:" << isContextValid()
             << "Sharing:" << isContextSharing();

    addRenderer<WaveformRenderBackground>();
    addRenderer<WaveformRendererEndOfTrack>();
    addRenderer<WaveformRendererPreroll>();
    addRenderer<WaveformRenderMarkRange>();
    addRenderer<QtWaveformRendererSimpleSignal>();
    addRenderer<WaveformRenderBeat>();
    addRenderer<WaveformRenderMark>();

    m_initSuccess = init();
}

QtSimpleWaveformWidget::~QtSimpleWaveformWidget() {
}

void QtSimpleWaveformWidget::castToQWidget() {
    m_widget = this;
}

void QtSimpleWaveformWidget::paintEvent(QPaintEvent* event) {
    //qDebug() << "paintEvent()";
    Q_UNUSED(event);
}

mixxx::Duration QtSimpleWaveformWidget::render() {
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
    //qDebug() << "QtSimpleWaveformWidget" << t1 << t2;
    return t1; // return timer for painter setup
}
