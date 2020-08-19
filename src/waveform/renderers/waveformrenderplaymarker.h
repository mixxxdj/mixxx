#pragma once

#include "waveform/renderers/waveformwidgetrenderer.h"

class WaveformRenderPlayMarker : public WaveformRendererAbstract {
  public:
    explicit WaveformRenderPlayMarker(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRenderPlayMarker();

    void setup(const QDomNode& node, const SkinContext& context) override;
    void draw(QPainter* painter, QPaintEvent* event) override;
    static QString generateBarBeatDisplayTextFromPrevBeat(mixxx::Beat beat);

  private:
    DISALLOW_COPY_AND_ASSIGN(WaveformRenderPlayMarker);
};