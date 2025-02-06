#pragma once

#include "waveform/renderers/waveformrenderbackground.h"
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
#include "waveform/renderers/deprecated/glwaveformrenderer.h"
#endif

class GLWaveformRenderBackground : public WaveformRenderBackground
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
        ,
                                   public GLWaveformRenderer
#endif
{
  public:
    explicit GLWaveformRenderBackground(
            WaveformWidgetRenderer* waveformWidgetRenderer);

#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
    virtual void draw(QPainter* painter, QPaintEvent* event);
#endif

  private:
    DISALLOW_COPY_AND_ASSIGN(GLWaveformRenderBackground);
};
