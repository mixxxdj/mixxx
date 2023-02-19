#pragma once

#include <QColor>
#include <QObject>
#include <QOpenGLShaderProgram>

#include "util/class.h"
#include "waveform/renderers/qopengl/waveformrenderer.h"
#include "waveform/renderers/waveformmarkset.h"

class QDomNode;
class SkinContext;
class QOpenGLTexture;

namespace qopengl {
class WaveformRenderMark;
}

class qopengl::WaveformRenderMark : public QObject, public qopengl::WaveformRenderer {
    Q_OBJECT
  public:
    explicit WaveformRenderMark(WaveformWidgetRenderer* waveformWidget);
    ~WaveformRenderMark() override;

    void setup(const QDomNode& node, const SkinContext& context) override;

    void initializeGL() override;
    void renderGL() override;
    void resizeGL(int w, int h) override;

    // Called when a new track is loaded.
    void onSetTrack() override;

  public slots:
    // Called when the loaded track's cues are added, deleted or modified and
    // when a new track is loaded.
    // It updates the marks' names and regenerates their image if needed.
    // This method is used for hotcues.
    void slotCuesUpdated();

  private:
    void checkCuesUpdated();

    void initGradientShader();
    void initTextureShader();

    void generateMarkImage(WaveformMarkPointer pMark);
    void generatePlayPosMarkTexture();

    void drawTriangle(QPainter* painter,
            const QBrush& fillColor,
            QPointF p1,
            QPointF p2,
            QPointF p3);

    WaveformMarkSet m_marks;
    QOpenGLShaderProgram m_gradientShaderProgram;
    QOpenGLShaderProgram m_textureShaderProgram;
    std::unique_ptr<QOpenGLTexture> m_pPlayPosMarkTexture;
    bool m_bCuesUpdates{};

    void fillRectWithGradient(const QRectF& rect, QColor color, Qt::Orientation orientation);
    void drawTexture(float x, float y, QOpenGLTexture* texture);

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderMark);
};
