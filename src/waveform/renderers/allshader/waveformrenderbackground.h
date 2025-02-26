#pragma once

#include <QColor>

#include "rendergraph/openglnode.h"
#include "util/class.h"
#include "waveform/renderers/allshader/waveformrenderer.h"

namespace allshader {
class WaveformRenderBackground;
} // namespace allshader

class allshader::WaveformRenderBackground final
        : public allshader::WaveformRenderer,
          public rendergraph::OpenGLNode {
  public:
    explicit WaveformRenderBackground(WaveformWidgetRenderer* waveformWidgetRenderer);

    void setup(const QDomNode& node, const SkinContext& skinContext) override;
    void paintGL() override;

  private:
    QColor m_backgroundColor;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderBackground);
};
