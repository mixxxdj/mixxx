#include "softwarewaveformwidget.h"

#include <QPainter>

#include "moc_softwarewaveformwidget.cpp"
#include "waveform/renderers/waveformrenderbackground.h"
#include "waveform/renderers/waveformrenderbeat.h"
#include "waveform/renderers/waveformrendererendoftrack.h"
#include "waveform/renderers/waveformrendererfilteredsignal.h"
#include "waveform/renderers/waveformrendererpreroll.h"
#include "waveform/renderers/waveformrendermark.h"
#include "waveform/renderers/waveformrendermarkrange.h"
#include "waveform/renderers/waveformwidgetrenderer.h"

SoftwareWaveformWidget::SoftwareWaveformWidget(const QString& group, QWidget* parent)
        : QWidget(parent),
          WaveformWidgetAbstract(group) {
    addRenderer<WaveformRenderBackground>();
    addRenderer<WaveformRendererEndOfTrack>();
    addRenderer<WaveformRendererPreroll>();
    addRenderer<WaveformRenderMarkRange>();
    addRenderer<WaveformRendererFilteredSignal>();
    addRenderer<WaveformRenderBeat>();
    addRenderer<WaveformRenderMark>();

    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);

    m_initSuccess = init();
}

SoftwareWaveformWidget::~SoftwareWaveformWidget() {
}

void SoftwareWaveformWidget::castToQWidget() {
    m_widget = this;
}

void SoftwareWaveformWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    draw(&painter, event);
}
