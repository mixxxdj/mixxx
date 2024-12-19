#include "waveform/renderers/allshader/waveformrenderersignalbase.h"

namespace allshader {

WaveformRendererSignalBase::WaveformRendererSignalBase(
        WaveformWidgetRenderer* waveformWidget)
        : ::WaveformRendererSignalBase(waveformWidget) {
}

void WaveformRendererSignalBase::draw(QPainter* painter, QPaintEvent* event) {
    Q_UNUSED(painter);
    Q_UNUSED(event);
    DEBUG_ASSERT(false);
}

} // namespace allshader
