#ifndef WAVEFORMRENDERERFILTEREDSIGNAL_H
#define WAVEFORMRENDERERFILTEREDSIGNAL_H

#include "waveformrendererabstract.h"

#include <QColor>
#include <QVector>
#include <QLineF>

class WaveformRendererFilteredSignal : public WaveformRendererAbstract
{
public:
    WaveformRendererFilteredSignal( WaveformWidgetRenderer* waveformWidget);

    virtual void init();
    virtual void setup( const QDomNode& node);
    virtual void draw( QPainter* painter, QPaintEvent* event);

    virtual void onResize();

protected:
    QColor m_signalColor;
    QColor m_lowColor;
    QColor m_midColor;
    QColor m_highColor;

    QVector<QLine> m_lowLines;
    QVector<QLine> m_midLines;
    QVector<QLine> m_highLines;
};

#endif // WAVEFORMRENDERERFILTEREDSIGNAL_H
