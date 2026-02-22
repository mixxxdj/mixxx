#include "waveform/widgets/allshader/waveformwidgettexturedstacked.h"

#include "waveform/renderers/allshader/waveformrenderbackground.h"
#include "waveform/renderers/allshader/waveformrenderbeat.h"
#include "waveform/renderers/allshader/waveformrendererendoftrack.h"
#include "waveform/renderers/allshader/waveformrendererpreroll.h"
#include "waveform/renderers/allshader/waveformrenderertextured.h"
#include "waveform/renderers/allshader/waveformrendermark.h"
#include "waveform/renderers/allshader/waveformrendermarkrange.h"
#include "waveform/widgets/allshader/moc_waveformwidgettexturedstacked.cpp"

namespace allshader {

WaveformWidgetTexturedStacked::WaveformWidgetTexturedStacked(const QString& group, QWidget* parent)
        : WaveformWidget(group, parent) {
    addRenderer<WaveformRenderBackground>();
    addRenderer<WaveformRendererEndOfTrack>();
    addRenderer<WaveformRendererPreroll>();
    addRenderer<WaveformRenderMarkRange>();
    addRenderer<WaveformRendererTextured>(WaveformRendererTextured::Type::Stacked);
    addRenderer<WaveformRenderBeat>();
    addRenderer<WaveformRenderMark>();

    m_initSuccess = init();
}

void WaveformWidgetTexturedStacked::castToQWidget() {
    m_widget = this;
}

void WaveformWidgetTexturedStacked::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
}

} // namespace allshader
