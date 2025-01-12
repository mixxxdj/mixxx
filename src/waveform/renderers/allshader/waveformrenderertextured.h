#pragma once

#ifndef QT_OPENGL_ES_2

#include "shaders/rgbshader.h"
#include "track/track_decl.h"
#include "util/class.h"
#include "waveform/renderers/allshader/rgbdata.h"
#include "waveform/renderers/allshader/vertexdata.h"
#include "waveform/renderers/allshader/waveformrenderersignalbase.h"
#include "waveform/waveform.h"
#include "waveform/widgets/waveformwidgettype.h"

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
    explicit WaveformRendererTextured(WaveformWidgetRenderer* waveformWidget,
            WaveformWidgetType::Type t,
            ::WaveformRendererAbstract::PositionSource type =
                    ::WaveformRendererAbstract::Play,
            WaveformRendererSignalBase::Options options =
                    WaveformRendererSignalBase::Option::None);
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
    struct WaveformTexture {
        unsigned char low;
        unsigned char mid;
        unsigned char high;
        unsigned char all;
    };

    static QString fragShaderForType(WaveformWidgetType::Type t);
    bool loadShaders();
    bool loadTexture();

    void createGeometry();
    void createFrameBuffers();

    GLint m_unitQuadListId;
    GLuint m_textureId;

    TrackPointer m_loadedTrack;
    int m_textureRenderedWaveformCompletion;

    std::vector<WaveformFilteredData> m_data;

    // Frame buffer for two pass rendering.
    std::unique_ptr<QOpenGLFramebufferObject> m_framebuffer;

    // shaders
    bool m_isSlipRenderer;
    WaveformRendererSignalBase::Options m_options;
    bool m_shadersValid;
    WaveformWidgetType::Type m_type;
    const QString m_pFragShader;
    std::unique_ptr<QOpenGLShaderProgram> m_frameShaderProgram;
};

#endif // QT_OPENGL_ES_2
