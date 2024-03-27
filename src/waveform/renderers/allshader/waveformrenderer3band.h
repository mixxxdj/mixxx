#pragma once

#include <vector>

#include "shaders/rgbashader.h"
#include "util/class.h"
#include "waveform/renderers/allshader/rgbadata.h"
#include "waveform/renderers/allshader/vertexdata.h"
#include "waveform/renderers/allshader/waveformrenderersignalbase.h"

namespace allshader {
class WaveformRendererThreeBand;
}

/// @brief Normaliser use to reduce noise and smooth curves on the waveform
class MovingAverageThreshold {
  public:
    MovingAverageThreshold()
            : MovingAverageThreshold(0.f, 0) {
    }
    MovingAverageThreshold(float threshold, int max);
    float add(float sample);
    void reset();

  private:
    std::vector<float> m_stack;
    float m_threshold;
    float m_currentAverage;
    int m_cursor;
    int m_size;
    int m_max;
};

class allshader::WaveformRendererThreeBand final : public allshader::WaveformRendererSignalBase {
  public:
    explicit WaveformRendererThreeBand(WaveformWidgetRenderer* waveformWidget);

    // override ::WaveformRendererSignalBase
    void onSetup(const QDomNode& node) override;

    void initializeGL() override;
    void paintGL() override;

  private:
    mixxx::RGBAShader m_shader;
    VertexData m_vertices;
    RGBAData m_colors;

    int m_lowSmoothFactor;
    int m_midSmoothFactor;
    int m_highSmoothFactor;

    float m_lowSmoothThreshold;
    float m_midSmoothThreshold;
    float m_highSmoothThreshold;

    MovingAverageThreshold m_normalisers[3];

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererThreeBand);
};
