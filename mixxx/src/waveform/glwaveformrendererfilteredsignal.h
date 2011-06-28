#ifndef GLWAVEFROMRENDERERFILTEREDSIGNAL_H
#define GLWAVEFROMRENDERERFILTEREDSIGNAL_H

#include "waveformrendererfilteredsignal.h"

#include <QBrush>

class ControlObject;

class GLWaveformRendererFilteredSignal : public WaveformRendererAbstract
{
public:
    GLWaveformRendererFilteredSignal( WaveformWidgetRenderer* waveformWidgetRenderer);

    virtual void init();
    virtual void setup(const QDomNode &node);
    virtual void draw(QPainter* painter, QPaintEvent* event);

private:
    ControlObject* m_lowFilterControlObject;
    ControlObject* m_midFilterControlObject;
    ControlObject* m_highFilterControlObject;

    ControlObject* m_lowKillControlObject;
    ControlObject* m_midKillControlObject;
    ControlObject* m_highKillControlObject;

    QColor m_signalColor;
    QBrush m_lowBrush;
    QBrush m_midBrush;
    QBrush m_highBrush;
    QBrush m_lowKilledBrush;
    QBrush m_midKilledBrush;
    QBrush m_highKilledBrush;
};

#endif // GLWAVEFROMRENDERERFILTEREDSIGNAL_H
