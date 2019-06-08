#ifndef GLWAVEFROMRENDERERFILTEREDSIGNAL_H
#define GLWAVEFROMRENDERERFILTEREDSIGNAL_H

#include <QOpenGLFunctions_2_1>

#include "waveformrenderersignalbase.h"

class ControlObject;

class GLWaveformRendererFilteredSignal: public WaveformRendererSignalBase,
        protected QOpenGLFunctions_2_1 {
public:
    explicit GLWaveformRendererFilteredSignal(
            WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~GLWaveformRendererFilteredSignal();

    virtual void onSetup(const QDomNode &node);
    virtual void draw(QPainter* painter, QPaintEvent* event);
};

#endif // GLWAVEFROMRENDERERFILTEREDSIGNAL_H
