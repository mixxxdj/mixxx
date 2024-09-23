#pragma once

#include "rendergraph/geometrynode.h"
#include "util/class.h"
#include "waveform/renderers/allshader/waveformrenderersignalbase.h"

namespace allshader {
class WaveformRendererRGB;
}

class allshader::WaveformRendererRGB final
        : public allshader::WaveformRendererSignalBase,
          public rendergraph::GeometryNode {
  public:
    explicit WaveformRendererRGB(WaveformWidgetRenderer* waveformWidget,
            ::WaveformRendererAbstract::PositionSource type =
                    ::WaveformRendererAbstract::Play,
            WaveformRendererSignalBase::Options options = WaveformRendererSignalBase::Option::None);

    // Pure virtual from WaveformRendererSignalBase, not used
    void onSetup(const QDomNode& node) override;

    bool supportsSlip() const override {
        return true;
    }

    // Virtuals for rendergraph::Node
    void preprocess() override;

  private:
    bool m_isSlipRenderer;
    WaveformRendererSignalBase::Options m_options;

    bool preprocessInner();

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererRGB);
};
