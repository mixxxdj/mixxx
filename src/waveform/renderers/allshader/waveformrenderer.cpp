#include "waveform/renderers/allshader/waveformrenderer.h"

#include "util/assert.h"

namespace allshader {

WaveformRenderer::WaveformRenderer(WaveformWidgetRenderer* widget)
        : ::WaveformRendererAbstract(widget) {
}

void WaveformRenderer::draw(QPainter*, QPaintEvent*) {
    DEBUG_ASSERT(false);
}

} // namespace allshader
