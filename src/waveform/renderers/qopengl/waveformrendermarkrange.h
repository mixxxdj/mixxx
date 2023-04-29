#pragma once

#include <QColor>
#include <vector>

#include "qopengl/shaders/unicolorshader.h"
#include "util/class.h"
#include "waveform/renderers/qopengl/waveformrenderer.h"
#include "waveform/renderers/waveformmarkrange.h"

class QDomNode;
class SkinContext;

namespace qopengl {
class WaveformRenderMarkRange;
}

class qopengl::WaveformRenderMarkRange final : public qopengl::WaveformRenderer {
  public:
    explicit WaveformRenderMarkRange(WaveformWidgetRenderer* waveformWidget);
    ~WaveformRenderMarkRange() override;

    void setup(const QDomNode& node, const SkinContext& context) override;

    void initializeGL() override;
    void renderGL() override;

  private:
    void fillRect(const QRectF& rect, QColor color);

    UnicolorShader m_shader;
    std::vector<WaveformMarkRange> m_markRanges;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderMarkRange);
};
