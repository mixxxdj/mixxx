#pragma once

#include <QOpenGLFunctions>

namespace allshader {
class WaveformRendererAbstract;
}

class allshader::WaveformRendererAbstract : public QOpenGLFunctions {
  public:
    /// This interface class is called by class allshader::WaveformWidget.
    /// Class allshader::WaveformWidget is derived from the allshader-based
    /// class WGLWidget and, in its implementation of the WGLWidget virtual
    /// methods, calls a stack of allshader::WaveformRendererAbstract instances, for
    /// the different layers of the waveform graphics (background, beat
    /// markers, the actual waveform, etc). In other word, this interface
    /// mimics the WGLWidget virtuals, but to be called as layers of an
    /// actual WGLWidget.
    virtual ~WaveformRendererAbstract() = default;
    virtual void initializeGL() {
        initializeOpenGLFunctions();
    }
    virtual void resizeGL(int /* w */, int /* h */) {
    }
    virtual void paintGL() = 0;
};
