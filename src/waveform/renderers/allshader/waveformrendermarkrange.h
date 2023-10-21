#pragma once

#include <QColor>
#include <vector>

#include "shaders/unicolorshader.h"
#include "util/class.h"
#include "waveform/renderers/allshader/waveformrenderer.h"
#include "waveform/renderers/waveformmarkrange.h"

class QDomNode;
class SkinContext;

namespace allshader {
class WaveformRenderMarkRange;
}

class allshader::WaveformRenderMarkRange final : public allshader::WaveformRenderer {
  public:
    explicit WaveformRenderMarkRange(WaveformWidgetRenderer* waveformWidget);

    void setup(const QDomNode& node, const SkinContext& context) override;

    void initializeGL() override;
    void paintGL() override;

  private:
    void fillRect(const QRectF& rect, QColor color);

    mixxx::UnicolorShader m_shader;
    std::vector<WaveformMarkRange> m_markRanges;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderMarkRange);
};
