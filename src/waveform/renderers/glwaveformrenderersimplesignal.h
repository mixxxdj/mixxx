#pragma once

#include "waveform/renderers/glwaveformrenderer.h"
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

#include "waveform/renderers/waveformrenderersignalbase.h"

class ControlObject;

class GLWaveformRendererSimpleSignal : public WaveformRendererSignalBase,
                                       public GLWaveformRenderer {
  public:
    explicit GLWaveformRendererSimpleSignal(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~GLWaveformRendererSimpleSignal();

    virtual void onSetup(const QDomNode &node);
    virtual void draw(QPainter* painter, QPaintEvent* event);
};

#endif // QT_NO_OPENGL && !QT_OPENGL_ES_2
