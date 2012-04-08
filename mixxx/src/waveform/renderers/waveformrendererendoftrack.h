#ifndef WAVEFORMRENDERERENDOFTRACK_H
#define WAVEFORMRENDERERENDOFTRACK_H

#include <QColor>
#include <QPixmap>
#include <QTime>

#include "util.h"
#include "waveformrendererabstract.h"

class WaveformRendererEndOfTrack : public WaveformRendererAbstract {
  public:
    static const int s_maxAlpha = 125;
    explicit WaveformRendererEndOfTrack(
        WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRendererEndOfTrack();

    virtual void init();
    virtual void setup(const QDomNode& node);
    virtual void draw(QPainter* painter, QPaintEvent* event);

  private:
    void generatePixmap();

    QColor m_color;
    QTime m_timer;
    int m_blinkingPeriodMillis;
    double m_remainingTimeTriggerSeconds;
    QPixmap m_pixmaps[s_maxAlpha];

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererEndOfTrack);
};

#endif // WAVEFORMRENDERERENDOFTRACK_H
