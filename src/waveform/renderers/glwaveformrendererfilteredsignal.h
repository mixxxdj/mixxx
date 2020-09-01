#pragma once

#include <QOpenGLFunctions_2_1>
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

#include <QDomNode>

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

#endif // QT_NO_OPENGL && !QT_OPENGL_ES_2
