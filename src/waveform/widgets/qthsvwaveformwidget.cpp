#include "waveform/widgets/qthsvwaveformwidget.h"

#include <QGLContext>
#include <QPainter>
#include <QtDebug>

#include "moc_qthsvwaveformwidget.cpp"
#include "waveform/renderers/waveformrenderbackground.h"
#include "waveform/renderers/waveformrenderbeat.h"
#include "waveform/renderers/waveformrendererendoftrack.h"
#include "waveform/renderers/waveformrendererhsv.h"
#include "waveform/renderers/waveformrendererpreroll.h"
#include "waveform/renderers/waveformrendermark.h"
#include "waveform/renderers/waveformrendermarkrange.h"
#include "waveform/renderers/waveformwidgetrenderer.h"

QtHSVWaveformWidget::QtHSVWaveformWidget(const QString& group, QWidget* parent)
        : QGLWidget(parent),
          WaveformWidgetAbstract(group) {
    if (QGLContext::currentContext() != context()) {
        makeCurrent();
    }
    addRenderer<WaveformRenderBackground>();
    addRenderer<WaveformRendererEndOfTrack>();
    addRenderer<WaveformRendererPreroll>();
    addRenderer<WaveformRenderMarkRange>();
    addRenderer<WaveformRendererHSV>();
    addRenderer<WaveformRenderBeat>();
    addRenderer<WaveformRenderMark>();

    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);

    m_initSuccess = init();
}

QtHSVWaveformWidget::~QtHSVWaveformWidget() {
}

void QtHSVWaveformWidget::castToQWidget() {
    m_widget = this;
}

void QtHSVWaveformWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
}

mixxx::Duration QtHSVWaveformWidget::render() {
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
    //qDebug() << "QtHSVWaveformWidget "<< t1 << t2;
    return t1; // return timer for painter setup
}
