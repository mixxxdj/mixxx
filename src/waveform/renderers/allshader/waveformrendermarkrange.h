#pragma once

#include <QColor>
#include <memory>

#include "rendergraph/node.h"
#include "util/class.h"
#include "waveform/renderers/waveformmarkrange.h"
#include "waveform/renderers/waveformrendererabstract.h"

class QDomNode;
class SkinContext;

namespace allshader {
class WaveformRenderMarkRange;
}

class allshader::WaveformRenderMarkRange final : public ::WaveformRendererAbstract,
                                                 public rendergraph::Node {
  public:
    explicit WaveformRenderMarkRange(WaveformWidgetRenderer* waveformWidget);

    // Pure virtual from WaveformRendererAbstract, not used
    void draw(QPainter* painter, QPaintEvent* event) override final;

    void setup(const QDomNode& node, const SkinContext& context) override;

    void updateNode();

  private:
    std::vector<WaveformMarkRange> m_markRanges;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderMarkRange);
};
