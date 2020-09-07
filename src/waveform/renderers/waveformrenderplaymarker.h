#pragma once

#include "waveform/renderers/waveformwidgetrenderer.h"

class WaveformRenderPlayMarker : public WaveformRendererAbstract {
  public:
    explicit WaveformRenderPlayMarker(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRenderPlayMarker();

    void setup(const QDomNode& node, const SkinContext& context) override;
    void draw(QPainter* painter, QPaintEvent* event) override;

  private:
    void drawTriangle(QPainter* painter,
                      QBrush fillColor,
                      QPointF p1,
                      QPointF p2,
                      QPointF p3);
    DISALLOW_COPY_AND_ASSIGN(WaveformRenderPlayMarker);
};
