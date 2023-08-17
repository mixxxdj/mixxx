#include "waveform/renderers/allshader/waveformrenderer.h"

#include "waveform/widgets/allshader/waveformwidget.h"

using namespace allshader;

WaveformRenderer::WaveformRenderer(WaveformWidgetRenderer* widget)
        : ::WaveformRendererAbstract(widget) {
}

void WaveformRenderer::draw(QPainter* painter, QPaintEvent* event) {
    DEBUG_ASSERT(false);
}
