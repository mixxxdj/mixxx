#ifndef WAVEFORMRENDERERHSV_H
#define WAVEFORMRENDERERHSV_H

#include "waveformrenderersignalbase.h"
#include "util.h"

class WaveformRendererHSV : public WaveformRendererSignalBase {
  public:
    explicit WaveformRendererHSV(
        WaveformWidgetRenderer* waveformWidget);
    virtual ~WaveformRendererHSV();

    virtual void onSetup(const QDomNode& node);

    virtual void draw(QPainter* painter, QPaintEvent* event);

  private:
    DISALLOW_COPY_AND_ASSIGN(WaveformRendererHSV);
};

#endif // WAVEFORMRENDERERFILTEREDSIGNAL_H
