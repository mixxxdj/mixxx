#include "waveform/renderers/allshader/waveformrenderersignalbase.h"

namespace allshader {

WaveformRendererSignalBase::WaveformRendererSignalBase(
        WaveformWidgetRenderer* waveformWidget)
        : ::WaveformRendererSignalBase(waveformWidget) {
}

void WaveformRendererSignalBase::draw(QPainter*, QPaintEvent*) {
    DEBUG_ASSERT(false);
}

} // namespace allshader
