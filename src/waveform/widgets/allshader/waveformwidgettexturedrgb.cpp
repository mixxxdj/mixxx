#include "waveform/widgets/allshader/waveformwidgettexturedrgb.h"

#include "waveform/renderers/allshader/waveformrenderbackground.h"
#include "waveform/renderers/allshader/waveformrenderbeat.h"
#include "waveform/renderers/allshader/waveformrendererendoftrack.h"
#include "waveform/renderers/allshader/waveformrendererpreroll.h"
#include "waveform/renderers/allshader/waveformrenderertextured.h"
#include "waveform/renderers/allshader/waveformrendermark.h"
#include "waveform/renderers/allshader/waveformrendermarkrange.h"
#include "waveform/widgets/allshader/moc_waveformwidgettexturedrgb.cpp"

namespace allshader {

WaveformWidgetTexturedRGB::WaveformWidgetTexturedRGB(const QString& group, QWidget* parent)
        : WaveformWidget(group, parent) {
    addRenderer<WaveformRenderBackground>();
    addRenderer<WaveformRendererEndOfTrack>();
    addRenderer<WaveformRendererPreroll>();
    addRenderer<WaveformRenderMarkRange>();
    addRenderer<WaveformRendererTextured>(WaveformRendererTextured::Type::RGB);
    addRenderer<WaveformRenderBeat>();
    addRenderer<WaveformRenderMark>();

    m_initSuccess = init();
}

void WaveformWidgetTexturedRGB::castToQWidget() {
    m_widget = this;
}

void WaveformWidgetTexturedRGB::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
}

} // namespace allshader
