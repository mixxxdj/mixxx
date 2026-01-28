#pragma once

#include "util/class.h"
#include "waveformrenderersignalbase.h"

class WaveformRendererSimpleSignal : public WaveformRendererSignalBase {
  public:
    explicit WaveformRendererSimpleSignal(
            WaveformWidgetRenderer* waveformWidget);
    virtual ~WaveformRendererSimpleSignal();

    virtual void onSetup(const QDomNode& node);
    virtual void draw(QPainter* painter, QPaintEvent* event);

  private:
    DISALLOW_COPY_AND_ASSIGN(WaveformRendererSimpleSignal);
};
