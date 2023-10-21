#pragma once

#include <QColor>
#include <QImage>

#include "shaders/patternshader.h"
#include "util/class.h"
#include "waveform/renderers/allshader/vertexdata.h"
#include "waveform/renderers/allshader/waveformrenderer.h"

class QDomNode;
class SkinContext;
class QOpenGLTexture;

namespace allshader {
class WaveformRendererPreroll;
}

class allshader::WaveformRendererPreroll final : public allshader::WaveformRenderer {
  public:
    explicit WaveformRendererPreroll(WaveformWidgetRenderer* waveformWidgetRenderer);
    ~WaveformRendererPreroll() override;

    void setup(const QDomNode& node, const SkinContext& context) override;
    void paintGL() override;
    void initializeGL() override;

  private:
    void drawPattern(float x1, float y1, float x2, float y2, float repetitions);

    mixxx::PatternShader m_shader;
    QColor m_color;
    float m_markerBreadth{};
    float m_markerLength{};
    std::unique_ptr<QOpenGLTexture> m_pTexture;

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererPreroll);
};
