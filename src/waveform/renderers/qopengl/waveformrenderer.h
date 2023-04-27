#pragma once

#include "waveform/renderers/qopengl/waveformrendererabstract.h"
#include "waveform/renderers/waveformrendererabstract.h"

class WaveformWidgetRenderer;

namespace qopengl {
class WaveformRenderer;
}

class qopengl::WaveformRenderer : public ::WaveformRendererAbstract,
                                  public qopengl::WaveformRendererAbstract {
  public:
    explicit WaveformRenderer(WaveformWidgetRenderer* widget);
    ~WaveformRenderer();

    // Pure virtual from qopengl::WaveformRendererAbstract.
    // Renderers that use QPainter functionality implement this.
    // But as all classes derived from qopengl::WaveformRenderer
    // will only use openGL functions (combining QPainter and
    // QOpenGLWindow has bad performance), we leave this empty.
    // Should never be called.
    void draw(QPainter* painter, QPaintEvent* event) override final;

    qopengl::WaveformRendererAbstract* qopenglWaveformRenderer() override final {
        // This class is indirectly derived from
        // WaveformWidgetRenderer, which has a member
        // QList<qopengl::WaveformRendererAbstract*> m_rendererStack;
        // In the case of qopengl::WaveformRenderer widgets,
        // all the items on this stack are derived from
        // qopengl::WaveformRendererAbstract and we use this method to
        // access them as such. (We could also have used a
        // dynamic cast (or even static cast instead)
        return this;
    }
};
