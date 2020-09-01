#pragma once

#include <QOpenGLFunctions_2_1>
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

#include "waveformrenderersignalbase.h"

class ControlObject;

class GLWaveformRendererRGB: public WaveformRendererSignalBase,
        protected QOpenGLFunctions_2_1 {
  public:
    explicit GLWaveformRendererRGB(
            WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~GLWaveformRendererRGB();

    virtual void onSetup(const QDomNode& node);
    virtual void draw(QPainter* painter, QPaintEvent* event);

  private:
    DISALLOW_COPY_AND_ASSIGN(GLWaveformRendererRGB);
};

#endif // QT_NO_OPENGL && !QT_OPENGL_ES_2
