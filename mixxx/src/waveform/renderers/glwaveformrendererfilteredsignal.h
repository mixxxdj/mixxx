#ifndef GLWAVEFROMRENDERERFILTEREDSIGNAL_H
#define GLWAVEFROMRENDERERFILTEREDSIGNAL_H

#include "waveformrenderersignalbase.h"

class ControlObject;

class GLWaveformRendererFilteredSignal : public WaveformRendererSignalBase {
  public:
    explicit GLWaveformRendererFilteredSignal( WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~GLWaveformRendererFilteredSignal();

    virtual void onInit();
    virtual void onSetup(const QDomNode &node);
    virtual void draw(QPainter* painter, QPaintEvent* event);
};

#endif // GLWAVEFROMRENDERERFILTEREDSIGNAL_H
