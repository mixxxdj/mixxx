#include "waveform/renderers/glwaveformrenderbackground.h"

GLWaveformRenderBackground::GLWaveformRenderBackground(
        WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRenderBackground(waveformWidgetRenderer) {
}

#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
void GLWaveformRenderBackground::draw(QPainter* painter, QPaintEvent* /*event*/) {
    painter->beginNativePainting();
    glClearColor(static_cast<float>(m_backgroundColor.redF()),
            static_cast<float>(m_backgroundColor.greenF()),
            static_cast<float>(m_backgroundColor.blueF()),
            1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    painter->endNativePainting();

    if (hasImage()) {
        drawImage(painter);
    }
}
#endif
