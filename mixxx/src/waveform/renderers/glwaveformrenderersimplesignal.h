#ifndef GLWAVEFORMRENDERERSIMPLESIGNAL_H
#define GLWAVEFORMRENDERERSIMPLESIGNAL_H

#include "waveformrenderersignalbase.h"

class ControlObject;

class GLWaveformRendererSimpleSignal : public WaveformRendererSignalBase {
public:
    explicit GLWaveformRendererSimpleSignal( WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~GLWaveformRendererSimpleSignal();

    virtual void onSetup(const QDomNode &node);
    virtual void draw(QPainter* painter, QPaintEvent* event);
};

#endif // GLWAVEFORMRENDERERSIMPLESIGNAL_H
