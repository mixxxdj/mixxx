#pragma once

#include <QColor>
#include <QImage>

#include "util/class.h"
#include "waveform/renderers/qopengl/waveformrenderer.h"

namespace qopengl {
class WaveformRenderBackground;
}
class qopengl::WaveformRenderBackground : public qopengl::WaveformRenderer {
  public:
    explicit WaveformRenderBackground(WaveformWidgetRenderer* waveformWidgetRenderer);
    ~WaveformRenderBackground() override;

    void setup(const QDomNode& node, const SkinContext& context) override;
    void renderGL() override;
  private:
    QColor m_backgroundColor;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderBackground);
};
