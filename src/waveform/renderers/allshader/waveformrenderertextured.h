#pragma once

#include "shaders/textureshader.h"
#include "track/track_decl.h"
#include "util/class.h"
#include "waveform/renderers/allshader/waveformrenderersignalbase.h"

class QOpenGLFramebufferObject;
class QOpenGLShaderProgram;

namespace allshader {
class WaveformRendererTextured;
}

// Based on GLSLWaveformRendererSignal (waveform/renderers/glslwaveformrenderersignal.h)
class allshader::WaveformRendererTextured : public QObject,
                                            public allshader::WaveformRendererSignalBase {
    Q_OBJECT
  public:
    enum class Type {
        Filtered,
        RGB,
        Stacked, // was RGBFiltered,
    };

    explicit WaveformRendererTextured(WaveformWidgetRenderer* waveformWidget, Type t);
    ~WaveformRendererTextured() override;

    // override ::WaveformRendererSignalBase
    void onSetup(const QDomNode& node) override;

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

    void onSetTrack() override;

  public slots:
    void slotWaveformUpdated();

  private:
    static QString fragShaderForType(Type t);
    bool loadShaders();
    bool loadTexture();

    void createFrameBuffers();

    GLuint m_textureId;

    TrackPointer m_loadedTrack;
    int m_textureRenderedWaveformCompletion;

    // Frame buffer for two pass rendering.
    std::unique_ptr<QOpenGLFramebufferObject> m_framebuffer;

    // shaders
    bool m_shadersValid;
    Type m_type;
    const QString m_pFragShader;
    std::unique_ptr<QOpenGLShaderProgram> m_frameShaderProgram;
    int m_matrixLocation;
    int m_positionLocation;
    int m_texcoordLocation;

    // Shader used to blit the rendered frame buffer onto the widget.
    mixxx::TextureShader m_textureShader;
};
