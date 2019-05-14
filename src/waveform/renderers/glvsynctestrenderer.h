#ifndef GLVSYNCTESTRENDERER_H
#define GLVSYNCTESTRENDERER_H

#include <QOpenGLFunctions_2_1>

#include "waveformrenderersignalbase.h"

class ControlObject;

class GLVSyncTestRenderer: public WaveformRendererSignalBase,
        protected QOpenGLFunctions_2_1 {
public:
    explicit GLVSyncTestRenderer(
            WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~GLVSyncTestRenderer();

    virtual void onSetup(const QDomNode &node);
    virtual void draw(QPainter* painter, QPaintEvent* event);
private:
    int m_drawcount;
};

#endif // GLVSYNCTESTRENDERER_H
