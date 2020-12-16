#pragma once

#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

#include "waveformrenderersignalbase.h"

class ControlObject;

class GLVSyncTestRenderer : public WaveformRendererSignalBase,
                            public GLWaveformRenderer {
  public:
    explicit GLVSyncTestRenderer(
            WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~GLVSyncTestRenderer();

    virtual void onSetup(const QDomNode &node);
    virtual void draw(QPainter* painter, QPaintEvent* event);
private:
    int m_drawcount;
};

#endif // QT_NO_OPENGL && !QT_OPENGL_ES_2
