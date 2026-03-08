#pragma once

#include <QColor>
#include <memory>

#include "skin/legacy/skincontext.h"
#include "util/class.h"
#include "util/performancetimer.h"
#include "waveform/renderers/waveformrendererabstract.h"

class ControlProxy;

class WaveformRendererEndOfTrack : public WaveformRendererAbstract {
  public:
    static constexpr int s_maxAlpha = 125;
    explicit WaveformRendererEndOfTrack(
            WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRendererEndOfTrack();

    virtual bool init();
    virtual void setup(const QDomNode& node, const SkinContext& context);
    virtual void onResize();
    virtual void draw(QPainter* painter, QPaintEvent* event);

  private:
    void generateBackRects();

    std::unique_ptr<ControlProxy> m_pEndOfTrackControl;
    std::unique_ptr<ControlProxy> m_pTimeRemainingControl;

    QColor m_color;
    PerformanceTimer m_timer;

    QVector<QRect> m_backRects;
    QPen m_pen;
    //QLinearGradient m_gradient;

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererEndOfTrack);
};
