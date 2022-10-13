#pragma once

#include "waveform/renderers/qopengl/waveformrenderersignalbase.h"

namespace qopengl {
class WaveformRendererRGB;
}

class qopengl::WaveformRendererRGB : public qopengl::WaveformRendererSignalBase {
  public:
    explicit WaveformRendererRGB(WaveformWidgetRenderer* waveformWidget);
    ~WaveformRendererRGB() override;

    // override ::WaveformRendererSignalBase
    void onSetup(const QDomNode& node) override;

    void initializeGL() override;
    void renderGL() override;

  private:
    int m_lineIndex;
    int m_colorIndex;

    QVector<float> m_lines;
    QVector<float> m_colors;

    void addRectangle(float x1, float y1, float x2, float y2, float r, float g, float b);

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererRGB);
};
