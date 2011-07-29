#ifndef WAVEFORMRENDERERENDOFTRACK_H
#define WAVEFORMRENDERERENDOFTRACK_H

#include <QColor>
#include <QPixmap>
#include <QTime>

#include "waveformrendererabstract.h"

class WaveformRendererEndOfTrack : public WaveformRendererAbstract
{
public:
    static const int s_maxAlpha = 125;

public:
    WaveformRendererEndOfTrack( WaveformWidgetRenderer* waveformWidgetRenderer);

    virtual void init();
    virtual void setup( const QDomNode& node);
    virtual void draw( QPainter* painter, QPaintEvent* event);

private:
    void generatePixmap();

    QColor m_color;
    QTime m_timer;
    int m_blinkingPeriod; //ms
    double m_remainingTimeTrigger; //s

    QPixmap m_pixmaps[s_maxAlpha];
};

#endif // WAVEFORMRENDERERENDOFTRACK_H
