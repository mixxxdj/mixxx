#include "softwarewaveformwidget.h"

#include <QPainter>

#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/renderers/waveformrenderbackground.h"
#include "waveform/renderers/waveformrendermark.h"
#include "waveform/renderers/waveformrendermarkrange.h"
#include "waveform/renderers/waveformrendererfilteredsignal.h"
#include "waveform/renderers/waveformrendererpreroll.h"
#include "waveform/renderers/waveformrendererendoftrack.h"
#include "waveform/renderers/waveformrenderbeat.h"

SoftwareWaveformWidget::SoftwareWaveformWidget( const char* group, QWidget* parent) :
    QWidget(parent),
    WaveformWidgetAbstract(group) {

    addRenderer<WaveformRenderBackground>();
    addRenderer<WaveformRendererEndOfTrack>();
    addRenderer<WaveformRendererPreroll>();
    addRenderer<WaveformRenderMarkRange>();
    addRenderer<WaveformRendererFilteredSignal>();
    addRenderer<WaveformRenderMark>();
    addRenderer<WaveformRenderBeat>();

    init();
}

SoftwareWaveformWidget::~SoftwareWaveformWidget() {
}

void SoftwareWaveformWidget::castToQWidget() {
    m_widget = static_cast<QWidget*>(this);
}

void SoftwareWaveformWidget::paintEvent( QPaintEvent* event) {
    QPainter painter(this);
    draw(&painter,event);
}

void SoftwareWaveformWidget::updateVisualSamplingPerPixel() {
    m_visualSamplePerPixel = m_zoomFactor;
    m_visualSamplePerPixel = math_max( 1.0, m_visualSamplePerPixel);
}
