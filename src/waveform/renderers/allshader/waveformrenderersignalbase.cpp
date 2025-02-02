#include "waveform/renderers/allshader/waveformrenderersignalbase.h"

namespace allshader {

WaveformRendererSignalBase::WaveformRendererSignalBase(
        WaveformWidgetRenderer* waveformWidget, const IVisualGainProvider* visualGainProvider)
        : ::WaveformRendererSignalBase(waveformWidget, visualGainProvider) {
}

void WaveformRendererSignalBase::draw(QPainter*, QPaintEvent*) {
    DEBUG_ASSERT(false);
}

} // namespace allshader
