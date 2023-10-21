#pragma once

#include <QColor>

#include "util/class.h"
#include "waveform/renderers/allshader/waveformrenderer.h"

namespace allshader {
class WaveformRenderBackground;
}

class allshader::WaveformRenderBackground final : public allshader::WaveformRenderer {
  public:
    explicit WaveformRenderBackground(WaveformWidgetRenderer* waveformWidgetRenderer);

    void setup(const QDomNode& node, const SkinContext& context) override;
    void paintGL() override;

  private:
    QColor m_backgroundColor;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderBackground);
};
