#ifndef WAVEFORMRENDERERRGB_H
#define WAVEFORMRENDERERRGB_H

#include "waveformrenderersignalbase.h"
#include "util.h"

class WaveformRendererRGB : public WaveformRendererSignalBase {
  public:
    explicit WaveformRendererRGB(
        WaveformWidgetRenderer* waveformWidget);
    virtual ~WaveformRendererRGB();

    virtual void onSetup(const QDomNode& node);
    virtual void draw(QPainter* painter, QPaintEvent* event);

  private:
    DISALLOW_COPY_AND_ASSIGN(WaveformRendererRGB);
};

#endif // WAVEFORMRENDERERRGB_H
