#pragma once

#include "waveform/renderers/qopengl/waveformrenderersignalbase.h"

namespace qopengl {
class WaveformRendererLRRGB;
}

class qopengl::WaveformRendererLRRGB : public qopengl::WaveformRendererSignalBase {
  public:
    explicit WaveformRendererLRRGB(WaveformWidgetRenderer* waveformWidget);
    ~WaveformRendererLRRGB() override;

    // override ::WaveformRendererSignalBase
    void onSetup(const QDomNode& node) override;

    void initializeGL() override;
    void renderGL() override;

  private:
    QVector<QVector2D> m_vertices;
    QVector<QVector3D> m_colors;

    void addRectangle(float x1, float y1, float x2, float y2, float r, float g, float b);

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererLRRGB);
};
