#pragma once

#include "glwaveformrenderersignal.h"
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

class GLWaveformRendererSimpleSignal : public GLWaveformRendererSignal {
  public:
    explicit GLWaveformRendererSimpleSignal(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~GLWaveformRendererSimpleSignal();

    virtual void onSetup(const QDomNode &node);
    virtual void draw(QPainter* painter, QPaintEvent* event);
};

#endif // QT_NO_OPENGL && !QT_OPENGL_ES_2
