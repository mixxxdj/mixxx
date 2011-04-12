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

private:
    QColor m_signalColor;
    QColor m_lowColor;
    QColor m_midColor;
    QColor m_highColor;

    QVector<QLineF> m_lowLines;
    QVector<QLineF> m_midLines;
    QVector<QLineF> m_highLines;
};

#endif // WAVEFORMRENDERERFILTEREDSIGNAL_H
