#pragma once

#include <QColor>
#include <QImage>
#include <memory>

#include "rendergraph/openglnode.h"
#include "shaders/patternshader.h"
#include "util/class.h"
#include "util/opengltexture2d.h"
#include "waveform/renderers/allshader/vertexdata.h"
#include "waveform/renderers/allshader/waveformrenderer.h"

class QDomNode;
class SkinContext;
class QOpenGLTexture;

namespace allshader {
class WaveformRendererPreroll;
}

class allshader::WaveformRendererPreroll final
        : public allshader::WaveformRenderer,
          public rendergraph::OpenGLNode {
  public:
    explicit WaveformRendererPreroll(
            WaveformWidgetRenderer* waveformWidget,
            ::WaveformRendererAbstract::PositionSource type =
                    ::WaveformRendererAbstract::Play);
    ~WaveformRendererPreroll() override;

    void setup(const QDomNode& node, const SkinContext& skinContext) override;
    void paintGL() override;
    void initializeGL() override;

  private:
    void drawPattern(float x1, float y1, float x2, float y2, float repetitions);

    mixxx::PatternShader m_shader;
    QColor m_color;
    float m_markerBreadth{};
    float m_markerLength{};
    OpenGLTexture2D m_texture;

    bool m_isSlipRenderer;

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererPreroll);
};
