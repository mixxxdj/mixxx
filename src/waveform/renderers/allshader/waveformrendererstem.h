#pragma once

#include <vector>

#include "shaders/rgbashader.h"
#include "shaders/textureshader.h"
#include "util/class.h"
#include "waveform/renderers/allshader/rgbadata.h"
#include "waveform/renderers/allshader/vertexdata.h"
#include "waveform/renderers/allshader/waveformrenderersignalbase.h"

class QOpenGLTexture;

namespace allshader {
class WaveformRendererStem;
}

class allshader::WaveformRendererStem final : public allshader::WaveformRendererSignalBase {
  public:
    explicit WaveformRendererStem(WaveformWidgetRenderer* waveformWidget,
            ::WaveformRendererAbstract::PositionSource type =
                    ::WaveformRendererAbstract::Play);

    // override ::WaveformRendererSignalBase
    void onSetup(const QDomNode& node) override;

    void initializeGL() override;
    void paintGL() override;

  private:
    mixxx::RGBAShader m_shader;
    mixxx::TextureShader m_textureShader;
    VertexData m_vertices;
    RGBAData m_colors;

    bool m_isSlipRenderer;

    std::vector<std::unique_ptr<ControlProxy>> m_pStemGain;
    std::vector<std::unique_ptr<ControlProxy>> m_pStemMute;

    void drawTexture(float x, float y, QOpenGLTexture* texture);

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererStem);
};
