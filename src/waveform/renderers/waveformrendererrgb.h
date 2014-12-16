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
    virtual void setup(const QDomNode& node, const SkinContext& context);
    virtual void draw(QPainter* painter, QPaintEvent* event);

  private:
    QColor m_lowColor;
    QColor m_midColor;
    QColor m_highColor;

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererRGB);
};

#endif // WAVEFORMRENDERERRGB_H
