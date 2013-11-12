#ifndef GLVSYNCTESTRENDERER_H
#define GLVSYNCTESTRENDERER_H

#include "waveformrenderersignalbase.h"

class ControlObject;

class GLVSyncTestRenderer : public WaveformRendererSignalBase {
  public:
    explicit GLVSyncTestRenderer( WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~GLVSyncTestRenderer();

    virtual void onSetup(const QDomNode &node);
    virtual void draw(QPainter* painter, QPaintEvent* event);
  private:
    int m_drawcount;
};

#endif // GLVSYNCTESTRENDERER_H
