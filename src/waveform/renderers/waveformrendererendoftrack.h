#ifndef WAVEFORMRENDERERENDOFTRACK_H
#define WAVEFORMRENDERERENDOFTRACK_H

#include <QColor>
#include <QTime>
//#include <QLinearGradient>

#include "skin/skincontext.h"
#include "util/class.h"
#include "waveform/renderers/waveformrendererabstract.h"
#include "waveform/waveformwidgetfactory.h"
#include "util/performancetimer.h"

class ControlObject;
class ControlProxy;

class WaveformRendererEndOfTrack : public WaveformRendererAbstract {
  public:
    static const int s_maxAlpha = 125;
    explicit WaveformRendererEndOfTrack(
            WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRendererEndOfTrack();

    virtual bool init();
    virtual void setup(const QDomNode& node, const SkinContext& context);
    virtual void onResize();
    virtual void draw(QPainter* painter, QPaintEvent* event);

  private:
    void generateBackRects();

    ControlProxy* m_pEndOfTrackControl;
    bool m_endOfTrackEnabled;
    ControlProxy* m_pTrackSampleRate;
    ControlProxy* m_pPlayControl;
    ControlProxy* m_pLoopControl;

    QColor m_color;
    PerformanceTimer m_timer;
    int m_remainingTimeTriggerSeconds;
    int m_blinkingPeriodMillis;

    QVector<QRect> m_backRects;
    QPen m_pen;
    //QLinearGradient m_gradient;

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererEndOfTrack);
};

#endif // WAVEFORMRENDERERENDOFTRACK_H
