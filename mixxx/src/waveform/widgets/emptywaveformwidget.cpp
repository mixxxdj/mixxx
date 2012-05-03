#include <QPainter>

#include "waveform/widgets/emptywaveformwidget.h"

#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/renderers/waveformrenderbackground.h"

EmptyWaveformWidget::EmptyWaveformWidget(const char* group, QWidget* parent)
        : WaveformWidgetAbstract(group),
          QWidget(parent) {
    //Empty means just a background ;)
    addRenderer<WaveformRenderBackground>();

    init();
}

EmptyWaveformWidget::~EmptyWaveformWidget() {
}

void EmptyWaveformWidget::castToQWidget() {
    m_widget = static_cast<QWidget*>(this);
}

void EmptyWaveformWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    draw(&painter,event);
}
