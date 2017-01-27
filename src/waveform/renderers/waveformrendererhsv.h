#ifndef WAVEFORMRENDERERHSV_H
#define WAVEFORMRENDERERHSV_H

#include "util/class.h"
#include "waveformrenderersignalbase.h"

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
