#pragma once

#include <vector>

#include "rendergraph/geometrynode.h"
#include "util/class.h"
#include "waveform/renderers/allshader/waveformrenderersignalbase.h"

class QOpenGLTexture;
class ControlProxy;

namespace allshader {
class WaveformRendererStem;
} // namespace allshader

class allshader::WaveformRendererStem final
        : public allshader::WaveformRendererSignalBase,
          public rendergraph::GeometryNode {
  public:
    explicit WaveformRendererStem(WaveformWidgetRenderer* waveformWidget,
            ::WaveformRendererAbstract::PositionSource type =
                    ::WaveformRendererAbstract::Play,
            ::WaveformRendererSignalBase::Options options =
                    ::WaveformRendererSignalBase::Option::None);

    // Pure virtual from WaveformRendererSignalBase, not used
    void onSetup(const QDomNode& node) override;

    bool init() override;

    bool supportsSlip() const override {
        return true;
    }

    // Virtuals for rendergraph::Node
    void preprocess() override;

  public slots:
    void setSplitStemTracks(bool splitStemTracks) {
        m_splitStemTracks = splitStemTracks;
    }
    void setReorderOnChange(bool value) {
        m_reorderOnChange = value;
        // Reset the stem layer stack to the natural order
        std::iota(m_stackOrder.begin(), m_stackOrder.end(), 0);
    }
    void setOutlineOpacity(float value) {
        m_outlineOpacity = value;
        markDirtyMaterial();
    }
    void setOpacity(float value) {
        m_opacity = value;
        markDirtyMaterial();
    }

  private:
    bool m_isSlipRenderer;
    bool m_splitStemTracks;

    bool m_reorderOnChange;
    float m_outlineOpacity;
    float m_opacity;

    std::vector<std::unique_ptr<ControlProxy>> m_pStemGain;
    std::vector<std::unique_ptr<ControlProxy>> m_pStemMute;

    QVarLengthArray<int, mixxx::kMaxSupportedStems> m_stackOrder;

    bool preprocessInner();

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererStem);
};
