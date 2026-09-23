#pragma once

#include <vector>

#include "rendergraph/geometrynode.h"
#include "util/class.h"
#include "waveform/renderers/allshader/waveformrenderersignalbase.h"

struct WaveformData;

namespace allshader {
class WaveformRendererRGB3Band;
} // namespace allshader

// Draws the low, mid and high bands as mirrored peak envelopes, similar to
// Rekordbox's 3-band waveform, with a separate color where low and mid overlap.
class allshader::WaveformRendererRGB3Band final
        : public allshader::WaveformRendererSignalBase,
          public rendergraph::GeometryNode {
  public:
    explicit WaveformRendererRGB3Band(WaveformWidgetRenderer* waveformWidget,
            ::WaveformRendererSignalBase::Options options);

    void onSetup(const QDomNode& node) override;

    // Virtuals for rendergraph::Node
    void preprocess() override;

  public slots:
    void setLowMidColor(const QColor& lowMidColor);

  private:
    bool preprocessInner();
    void calculateHeights(const WaveformData* data,
            int visualFramesSize,
            double visualSampleRate,
            double firstPixelVisualFrame,
            double visualIncrementPerPixel,
            int pixelLength,
            const float bandScale[3],
            float maxHeight);

    float m_lowMidColor_r;
    float m_lowMidColor_g;
    float m_lowMidColor_b;

    // Per-frame band envelopes and per-pixel layer heights
    std::vector<float> m_envelopes[3];
    std::vector<float> m_heights[4];

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererRGB3Band);
};
