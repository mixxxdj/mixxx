#pragma once

#include <QColor>
#include <vector>

#include "preferences/usersettings.h"
#include "waveform/renderers/qopengl/waveformshaderrenderer.h"
#include "waveform/renderers/waveformmarkrange.h"

class QDomNode;
class SkinContext;

namespace qopengl {
class WaveformRenderMarkRange;
}

class qopengl::WaveformRenderMarkRange : public qopengl::WaveformShaderRenderer {
  public:
    explicit WaveformRenderMarkRange(WaveformWidgetRenderer* waveformWidget);
    ~WaveformRenderMarkRange() override;

    void setup(const QDomNode& node, const SkinContext& context) override;

    void initializeGL() override;
    void renderGL() override;

  private:
    void fillRect(const QRectF& rect, QColor color);

    std::vector<WaveformMarkRange> m_markRanges;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderMarkRange);
};
