#include "waveform/widgets/allshader/rgbwaveformwidget.h"

#include "waveform/renderers/allshader/waveformrenderbackground.h"
#include "waveform/renderers/allshader/waveformrenderbeat.h"
#include "waveform/renderers/allshader/waveformrendererendoftrack.h"
#include "waveform/renderers/allshader/waveformrendererpreroll.h"
#include "waveform/renderers/allshader/waveformrendererrgb.h"
#include "waveform/renderers/allshader/waveformrendererslipmode.h"
#include "waveform/renderers/allshader/waveformrendermark.h"
#include "waveform/renderers/allshader/waveformrendermarkrange.h"
#include "waveform/widgets/allshader/moc_rgbwaveformwidget.cpp"

namespace allshader {

RGBWaveformWidget::RGBWaveformWidget(const QString& group, QWidget* parent)
        : WaveformWidget(group, parent) {
    addRenderer<WaveformRenderBackground>();
    addRenderer<WaveformRendererEndOfTrack>();
    addRenderer<WaveformRendererPreroll>();
    addRenderer<WaveformRenderMarkRange>();
    addRenderer<WaveformRendererRGB>();
    addRenderer<WaveformRenderBeat>();
    addRenderer<WaveformRenderMark>();
    // The following renderer will add an overlay waveform if a slip is in progress
    addRenderer<WaveformRendererSlipMode>();
    addRenderer<WaveformRendererPreroll>(::WaveformRendererAbstract::Slip);
    addRenderer<WaveformRendererRGB>(::WaveformRendererAbstract::Slip);
    addRenderer<WaveformRenderBeat>(::WaveformRendererAbstract::Slip);
    addRenderer<WaveformRenderMark>(::WaveformRendererAbstract::Slip);

    m_initSuccess = init();
}

void RGBWaveformWidget::castToQWidget() {
    m_widget = this;
}

void RGBWaveformWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
}

} // namespace allshader
