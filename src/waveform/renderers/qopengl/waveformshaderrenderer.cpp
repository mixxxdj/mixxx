#include "waveform/renderers/qopengl/waveformshaderrenderer.h"

using namespace qopengl;

WaveformShaderRenderer::WaveformShaderRenderer(WaveformWidgetRenderer* widget)
        : qopengl::WaveformRenderer(widget) {
}

WaveformShaderRenderer::~WaveformShaderRenderer() = default;
