#include "waveform/renderers/allshader/waveformrenderer.h"

#include "waveform/widgets/allshader/waveformwidget.h"

using namespace allshader;

WaveformRenderer::WaveformRenderer(WaveformWidgetRenderer* widget)
        : ::WaveformRendererAbstract(widget) {
}

WaveformRenderer::~WaveformRenderer() = default;

void WaveformRenderer::draw(QPainter* painter, QPaintEvent* event) {
    DEBUG_ASSERT(false);
}
