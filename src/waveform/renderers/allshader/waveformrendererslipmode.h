#pragma once

#include <QColor>
#include <memory>

#include "rendergraph/openglnode.h"
#include "shaders/slipmodeshader.h"
#include "util/class.h"
#include "util/performancetimer.h"
#include "waveform/renderers/allshader/waveformrenderer.h"

class ControlProxy;
class QDomNode;
class SkinContext;

namespace allshader {
class WaveformRendererSlipMode;
}

class allshader::WaveformRendererSlipMode final
        : public allshader::WaveformRenderer,
          public rendergraph::OpenGLNode {
  public:
    explicit WaveformRendererSlipMode(
            WaveformWidgetRenderer* waveformWidget);

    void setup(const QDomNode& node, const SkinContext& skinContext) override;

    bool init() override;

    void initializeGL() override;
    void paintGL() override;

  private:
    mixxx::SlipModeShader m_shader;
    std::unique_ptr<ControlProxy> m_pSlipMode;

    float m_slipBorderTopOutlineSize;
    float m_slipBorderBottomOutlineSize;

    QColor m_color;
    PerformanceTimer m_timer;

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererSlipMode);
};
