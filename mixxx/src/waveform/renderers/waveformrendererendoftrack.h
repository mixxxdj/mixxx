#ifndef WAVEFORMRENDERERENDOFTRACK_H
#define WAVEFORMRENDERERENDOFTRACK_H

#include <QColor>
#include <QPixmap>
#include <QTime>

#include "util.h"
#include "waveformrendererabstract.h"

class ControlObject;
class ControlObjectThreadMain;

class WaveformRendererEndOfTrack : public WaveformRendererAbstract {
public:
    static const int s_maxAlpha = 125;
    explicit WaveformRendererEndOfTrack(
            WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRendererEndOfTrack();

    virtual void init();
    virtual void setup(const QDomNode& node);
    virtual void onResize();
    virtual void draw(QPainter* painter, QPaintEvent* event);

private:
    ControlObjectThreadMain*  m_endOfTrackControl;
    bool m_endOfTrackEnabled;

    ControlObjectThreadMain* m_trackSampleRate;
    ControlObjectThreadMain* m_playControl;
    ControlObjectThreadMain* m_loopControl;

    QColor m_color;
    QTime m_timer;
    int m_blinkingPeriodMillis;
    double m_remainingTimeTriggerSeconds;

    QRect m_rect;
    QVector<QRect> m_backRects;
    QPen m_pen;

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererEndOfTrack);
};

#endif // WAVEFORMRENDERERENDOFTRACK_H
