#pragma once

#include <QDomNode>
#include <QGLContext>
#include <QOpenGLFunctions_2_1>
#include <QPaintEvent>
#include <QPainter>

#include "skin/skincontext.h"

class WaveformWidgetRenderer;

class WaveformRendererAbstract {
  public:
    explicit WaveformRendererAbstract(
            WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~WaveformRendererAbstract();

    virtual bool init() {return true; }
    virtual void setup(const QDomNode& node, const SkinContext& context) = 0;
    virtual void draw(QPainter* painter, QPaintEvent* event) = 0;

    virtual void onResize() {}
    virtual void onSetTrack() {}

  protected:
    bool isDirty() const {
        return m_dirty;
    }
    void setDirty(bool dirty = true) {
        m_dirty = dirty;
    }

    double scaleFactor() const {
        return m_scaleFactor;
    }
    void setScaleFactor(double scaleFactor) {
        m_scaleFactor = scaleFactor;
    }

    WaveformWidgetRenderer* m_waveformRenderer;

  private:

    bool m_dirty;
    double m_scaleFactor;

    friend class WaveformWidgetRenderer;
};

/// GLWaveformRenderer is a WaveformRendererAbstract which directly calls OpenGL functions.
///
/// Note that the Qt OpenGL WaveformRendererAbstracts are not GLWaveformRenderers because
/// they do not call OpenGL functions directly. Instead, they inherit QGLWidget and use the
/// QPainter API which Qt translates to OpenGL under the hood.
class GLWaveformRenderer : protected QOpenGLFunctions_2_1 {
  public:
    virtual void onInitializeGL() {
        initializeOpenGLFunctions();
    }

  protected:
    // Somehow QGLWidget does not call QGLWidget::initializeGL on macOS, so hack around that
    // by calling this in `draw` when the QGLContext has been made current.
    // TODO: remove this when upgrading to QOpenGLWidget
    void maybeInitializeGL() {
        if (QGLContext::currentContext() != m_pLastContext) {
            onInitializeGL();
            m_pLastContext = QGLContext::currentContext();
        }
    }

  private:
    const QGLContext* m_pLastContext;
};
