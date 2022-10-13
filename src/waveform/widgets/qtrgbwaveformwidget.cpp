#include "waveform/widgets/qtrgbwaveformwidget.h"

#include <QGLContext>
#include <QPainter>
#include <QtDebug>

#include "moc_qtrgbwaveformwidget.cpp"
#include "waveform/renderers/waveformrenderbackground.h"
#include "waveform/renderers/waveformrenderbeat.h"
#include "waveform/renderers/waveformrendererendoftrack.h"
#include "waveform/renderers/waveformrendererpreroll.h"
#include "waveform/renderers/waveformrendererrgb.h"
#include "waveform/renderers/waveformrendermark.h"
#include "waveform/renderers/waveformrendermarkrange.h"
#include "waveform/renderers/waveformwidgetrenderer.h"

QtRGBWaveformWidget::QtRGBWaveformWidget(const QString& group, QWidget* parent)
        : GLWaveformWidgetAbstract(group, parent) {
    qDebug() << "Created WGLWidget. Context"
             << "Valid:" << isContextValid()
             << "Sharing:" << isContextSharing();

    addRenderer<WaveformRenderBackground>();
    addRenderer<WaveformRendererEndOfTrack>();
    addRenderer<WaveformRendererPreroll>();
    addRenderer<WaveformRenderMarkRange>();
    addRenderer<WaveformRendererRGB>();
    addRenderer<WaveformRenderBeat>();
    addRenderer<WaveformRenderMark>();

    m_initSuccess = init();
}

QtRGBWaveformWidget::~QtRGBWaveformWidget() {
}

void QtRGBWaveformWidget::castToQWidget() {
    m_widget = this;
}

void QtRGBWaveformWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
}

mixxx::Duration QtRGBWaveformWidget::render() {
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
    //qDebug() << "GLVSyncTestWidget "<< t1 << t2;
    return t1; // return timer for painter setup
}
