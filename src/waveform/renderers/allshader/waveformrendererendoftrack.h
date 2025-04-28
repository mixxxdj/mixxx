#pragma once

#include <QColor>
#include <memory>

#include "rendergraph/geometrynode.h"
#include "rendergraph/opacitynode.h"
#include "util/class.h"
#include "util/performancetimer.h"
#include "waveform/renderers/waveformrendererabstract.h"

class ControlProxy;
class QDomNode;
class SkinContext;

namespace allshader {
class WaveformRendererEndOfTrack;
} // namespace allshader

class allshader::WaveformRendererEndOfTrack final
        : public QObject,
          public ::WaveformRendererAbstract,
          public rendergraph::GeometryNode {
    Q_OBJECT
  public:
    explicit WaveformRendererEndOfTrack(
            WaveformWidgetRenderer* waveformWidget);

    // Pure virtual from WaveformRendererAbstract, not used
    void draw(QPainter* painter, QPaintEvent* event) override final;

    void setup(const QDomNode& node, const SkinContext& skinContext) override;

    bool init() override;

    // Virtual for rendergraph::Node
    void preprocess() override;

  public slots:
    void setColor(const QColor& color) {
        m_color = color;
    }
    void setEndOfTrackWarningTime(int endOfTrackWarningTime) {
        m_remainingTimeTriggerSeconds = endOfTrackWarningTime;
    }

  private:
    std::unique_ptr<ControlProxy> m_pEndOfTrackControl;
    std::unique_ptr<ControlProxy> m_pTimeRemainingControl;

    QColor m_color;
    int m_remainingTimeTriggerSeconds;
    PerformanceTimer m_timer;

    bool preprocessInner();

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererEndOfTrack);
};
