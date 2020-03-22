#ifndef QTVSYNCTESTRENDERER_H
#define QTVSYNCTESTRENDERER_H

#include "waveformrenderersignalbase.h"

class ControlObject;

class QtVSyncTestRenderer : public WaveformRendererSignalBase {
  public:
    explicit QtVSyncTestRenderer(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~QtVSyncTestRenderer();

    virtual void onSetup(const QDomNode &node);
    virtual void draw(QPainter* painter, QPaintEvent* event);
  private:
    int m_drawcount;
};

#endif // QTVSYNCTESTRENDERER_H
