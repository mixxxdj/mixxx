#include "waveform/widgets/allshader/3bandwaveformwidget.h"

#include "waveform/renderers/allshader/waveformrenderbackground.h"
#include "waveform/renderers/allshader/waveformrenderbeat.h"
#include "waveform/renderers/allshader/waveformrenderer3band.h"
#include "waveform/renderers/allshader/waveformrendererendoftrack.h"
#include "waveform/renderers/allshader/waveformrendererpreroll.h"
#include "waveform/renderers/allshader/waveformrendermark.h"
#include "waveform/renderers/allshader/waveformrendermarkrange.h"
#include "waveform/widgets/allshader/moc_3bandwaveformwidget.cpp"

namespace allshader {

ThreeBandWaveformWidget::ThreeBandWaveformWidget(const QString& group, QWidget* parent)
        : WaveformWidget(group, parent) {
    addRenderer<WaveformRenderBackground>();
    addRenderer<WaveformRendererEndOfTrack>();
    addRenderer<WaveformRendererPreroll>();
    addRenderer<WaveformRenderMarkRange>();
    addRenderer<WaveformRendererThreeBand>();
    addRenderer<WaveformRenderBeat>();
    addRenderer<WaveformRenderMark>();

    m_initSuccess = init();
}

void ThreeBandWaveformWidget::castToQWidget() {
    m_widget = this;
}

void ThreeBandWaveformWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
}

} // namespace allshader
