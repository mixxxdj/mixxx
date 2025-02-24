#pragma once

#include <vector>

#include "control/pollingcontrolproxy.h"
#include "rendergraph/geometrynode.h"
#include "util/class.h"
#include "waveform/renderers/allshader/waveformrenderersignalbase.h"

class QOpenGLTexture;

namespace allshader {
class WaveformRendererStem;
} // namespace allshader

class allshader::WaveformRendererStem final
        : public allshader::WaveformRendererSignalBase,
          public rendergraph::GeometryNode {
  public:
    explicit WaveformRendererStem(WaveformWidgetRenderer* waveformWidget,
            ::WaveformRendererAbstract::PositionSource type =
                    ::WaveformRendererAbstract::Play);

    // Pure virtual from WaveformRendererSignalBase, not used
    void onSetup(const QDomNode& node) override;

    bool init() override;

    bool supportsSlip() const override {
        return true;
    }

    // Virtuals for rendergraph::Node
    void preprocess() override;

  private:
    bool m_isSlipRenderer;

    std::vector<std::unique_ptr<PollingControlProxy>> m_pStemGain;
    std::vector<std::unique_ptr<PollingControlProxy>> m_pStemMute;

    bool preprocessInner();

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererStem);
};
