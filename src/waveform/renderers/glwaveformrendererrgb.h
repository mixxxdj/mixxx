#ifndef GLWAVEFORMRENDERERRGB_H
#define GLWAVEFORMRENDERERRGB_H

#include "waveformrenderersignalbase.h"

class ControlObject;

class GLWaveformRendererRGB : public WaveformRendererSignalBase {
  public:
    explicit GLWaveformRendererRGB(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~GLWaveformRendererRGB();

    virtual void onSetup(const QDomNode& node);
    virtual void draw(QPainter* painter, QPaintEvent* event);

  private:
    DISALLOW_COPY_AND_ASSIGN(GLWaveformRendererRGB);
};

#endif // GLWAVEFORMRENDERERRGB_H
