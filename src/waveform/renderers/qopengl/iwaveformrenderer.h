#pragma once

#include <QOpenGLFunctions>

namespace qopengl {
class IWaveformRenderer;
}

class qopengl::IWaveformRenderer : public QOpenGLFunctions {
  public:
    /// This interface class is called by class qopengl::WaveformWidget.
    /// Class qopengl::WaveformWidget is derived from the qopengl-based
    /// class WGLWidget and, in its implementation of the WGLWidget virtual
    /// methods, calls a stack of IWaveformRenderer instances, for
    /// the different layers of the waveform graphics (background, beat
    /// markers, the actual waveform, etc). In other word, this interface
    /// mimics the WGLWidget virtuals, but to be called as layers of an
    /// actual WGLWidget.
    virtual ~IWaveformRenderer() = default;
    virtual void initializeGL() {
    }
    virtual void resizeGL(int w, int h) {
    }
    virtual void renderGL() = 0;
};
