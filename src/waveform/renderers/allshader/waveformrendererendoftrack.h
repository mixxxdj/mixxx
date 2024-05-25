#pragma once

#include <QColor>
#include <memory>

#include "shaders/endoftrackshader.h"
#include "util/class.h"
#include "util/performancetimer.h"
#include "waveform/renderers/allshader/waveformrenderer.h"

class ControlProxy;
class QDomNode;
class SkinContext;

namespace allshader {
class WaveformRendererEndOfTrack;
}

class allshader::WaveformRendererEndOfTrack final : public allshader::WaveformRenderer {
  public:
    explicit WaveformRendererEndOfTrack(
            WaveformWidgetRenderer* waveformWidget);

    void setup(const QDomNode& node, const SkinContext& context) override;

    bool init() override;

    void initializeGL() override;
    void paintGL() override;

  private:
    void fillWithGradient(QColor color);

    mixxx::EndOfTrackShader m_shader;
    std::unique_ptr<ControlProxy> m_pEndOfTrackControl;
    std::unique_ptr<ControlProxy> m_pTimeRemainingControl;

    QColor m_color;
    PerformanceTimer m_timer;

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererEndOfTrack);
};
