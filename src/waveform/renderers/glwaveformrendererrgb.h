#ifndef GLWAVEFORMRENDERERRGB_H
#define GLWAVEFORMRENDERERRGB_H

#include "waveformrenderersignalbase.h"

class ControlObject;

class GLWaveformRendererRGB : public WaveformRendererSignalBase {
public:
    explicit GLWaveformRendererRGB( WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~GLWaveformRendererRGB();

    virtual void onSetup(const QDomNode& node);
    virtual void setup(const QDomNode& node, const SkinContext& context);
    virtual void draw(QPainter* painter, QPaintEvent* event);

private:
    QColor m_lowColor;
    QColor m_midColor;
    QColor m_highColor;
};

#endif // GLWAVEFORMRENDERERRGB_H
