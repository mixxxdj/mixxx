#ifndef GLWAVEFORMRENDERERRGB_H
#define GLWAVEFORMRENDERERRGB_H

#include "waveformrenderersignalbase.h"

class ControlObject;

class GLWaveformRendererRGB : public WaveformRendererSignalBase {
public:
    explicit GLWaveformRendererRGB(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~GLWaveformRendererRGB();

    virtual void onSetup(const QDomNode& node);
    virtual void setup(const QDomNode& node, const SkinContext& context);
    virtual void draw(QPainter* painter, QPaintEvent* event);

private:
    QColor m_lowColor;
    QColor m_midColor;
    QColor m_highColor;
    qreal m_lowColor_r, m_lowColor_g, m_lowColor_b;
    qreal m_midColor_r, m_midColor_g, m_midColor_b;
    qreal m_highColor_r, m_highColor_g, m_highColor_b;
};

#endif // GLWAVEFORMRENDERERRGB_H
