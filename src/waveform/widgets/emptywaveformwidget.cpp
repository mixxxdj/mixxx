#include "waveform/widgets/emptywaveformwidget.h"

#include <QPainter>

#include "moc_emptywaveformwidget.cpp"
#include "waveform/renderers/waveformrenderbackground.h"
#include "waveform/renderers/waveformwidgetrenderer.h"

EmptyWaveformWidget::EmptyWaveformWidget(const QString& group, QWidget* parent)
        : QWidget(parent),
          WaveformWidgetAbstract(group) {
    //Empty means just a background ;)
    addRenderer<WaveformRenderBackground>();

    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);

    m_initSuccess = init();
}

EmptyWaveformWidget::~EmptyWaveformWidget() {
}

void EmptyWaveformWidget::castToQWidget() {
    m_widget = this;
}

void EmptyWaveformWidget::paintEvent(QPaintEvent* event) {
    // Only render if Qt thinks it is required
    QPainter painter(this);
    draw(&painter,event);
}

mixxx::Duration EmptyWaveformWidget::render() {
    // skip update every frame
    return mixxx::Duration();
}
