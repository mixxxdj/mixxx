#pragma once

#include <QOpenGLFunctions>

/// Interface for QOpenGL-based waveform renderers

namespace qopengl {
class IWaveformRenderer;
}

class qopengl::IWaveformRenderer : public QOpenGLFunctions {
  public:
    virtual void initializeGL() {
    }
    virtual void resizeGL(int w, int h) {
    }
    virtual void renderGL() = 0;
};
