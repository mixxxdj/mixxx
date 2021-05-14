#include "waveform/widgets/qtwaveformwidget.h"

#include <QGLContext>
#include <QPainter>
#include <QtDebug>

#include "moc_qtwaveformwidget.cpp"
#include "util/performancetimer.h"
#include "waveform/renderers/qtwaveformrendererfilteredsignal.h"
#include "waveform/renderers/waveformrenderbackground.h"
#include "waveform/renderers/waveformrenderbeat.h"
#include "waveform/renderers/waveformrendererendoftrack.h"
#include "waveform/renderers/waveformrendererpreroll.h"
#include "waveform/renderers/waveformrendermark.h"
#include "waveform/renderers/waveformrendermarkrange.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/sharedglcontext.h"

QtWaveformWidget::QtWaveformWidget(const QString& group, QWidget* parent)
        : QGLWidget(parent, SharedGLContext::getWidget()),
          WaveformWidgetAbstract(group) {
    qDebug() << "Created QGLWidget. Context"
             << "Valid:" << context()->isValid()
             << "Sharing:" << context()->isSharing();

    addRenderer<WaveformRenderBackground>();
    addRenderer<WaveformRendererEndOfTrack>();
    addRenderer<WaveformRendererPreroll>();
    addRenderer<WaveformRenderMarkRange>();
    addRenderer<QtWaveformRendererFilteredSignal>();
    addRenderer<WaveformRenderBeat>();
    addRenderer<WaveformRenderMark>();

    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);

    setAutoBufferSwap(false);

    m_initSuccess = init();
}

QtWaveformWidget::~QtWaveformWidget() {
}

void QtWaveformWidget::castToQWidget() {
    m_widget = this;
}

void QtWaveformWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
}

mixxx::Duration QtWaveformWidget::render() {
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
