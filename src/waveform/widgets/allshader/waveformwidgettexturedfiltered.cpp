#include "waveform/widgets/allshader/waveformwidgettexturedfiltered.h"

#include "waveform/renderers/allshader/waveformrenderbackground.h"
#include "waveform/renderers/allshader/waveformrenderbeat.h"
#include "waveform/renderers/allshader/waveformrendererendoftrack.h"
#include "waveform/renderers/allshader/waveformrendererpreroll.h"
#include "waveform/renderers/allshader/waveformrenderertextured.h"
#include "waveform/renderers/allshader/waveformrendermark.h"
#include "waveform/renderers/allshader/waveformrendermarkrange.h"
#include "waveform/widgets/allshader/moc_waveformwidgettexturedfiltered.cpp"

namespace allshader {

WaveformWidgetTexturedFiltered::WaveformWidgetTexturedFiltered(
        const QString& group, QWidget* parent)
        : WaveformWidget(group, parent) {
    addRenderer<WaveformRenderBackground>();
    addRenderer<WaveformRendererEndOfTrack>();
    addRenderer<WaveformRendererPreroll>();
    addRenderer<WaveformRenderMarkRange>();
    addRenderer<WaveformRendererTextured>(WaveformRendererTextured::Type::Filtered);
    addRenderer<WaveformRenderBeat>();
    addRenderer<WaveformRenderMark>();

    m_initSuccess = init();
}

void WaveformWidgetTexturedFiltered::castToQWidget() {
    m_widget = this;
}

void WaveformWidgetTexturedFiltered::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
}

} // namespace allshader
