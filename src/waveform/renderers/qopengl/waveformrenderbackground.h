#pragma once

#include <QColor>
#include <QImage>

#include "util/class.h"
#include "waveform/renderers/qopengl/waveformrenderer.h"

class QDomNode;
class SkinContext;

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
    void generateImage();

    QString m_backgroundPixmapPath;
    QColor m_backgroundColor;
    QImage m_backgroundImage;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderBackground);
};
