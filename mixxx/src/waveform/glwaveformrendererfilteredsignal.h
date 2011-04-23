#ifndef GLWAVEFROMRENDERERFILTEREDSIGNAL_H
#define GLWAVEFROMRENDERERFILTEREDSIGNAL_H

#include "waveformrendererfilteredsignal.h"

#include <QBrush>

class GLWaveformRendererFilteredSignal : public WaveformRendererFilteredSignal
{
public:
    GLWaveformRendererFilteredSignal( WaveformWidgetRenderer* waveformWidget);
    virtual void setup(const QDomNode &node);
    virtual void draw(QPainter* painter, QPaintEvent* event);



private:
    QBrush m_lowBrush;
    QBrush m_midBrush;
    QBrush m_highBrush;
};

#endif // GLWAVEFROMRENDERERFILTEREDSIGNAL_H
