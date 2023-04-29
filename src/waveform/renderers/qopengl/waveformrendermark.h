#pragma once

#include <QColor>
#include <QObject>

#include "qopengl/shaders/rgbashader.h"
#include "qopengl/shaders/textureshader.h"
#include "util/class.h"
#include "waveform/renderers/qopengl/waveformrenderer.h"
#include "waveform/renderers/waveformmarkset.h"

class QDomNode;
class SkinContext;
class QOpenGLTexture;

namespace qopengl {
class WaveformRenderMark;
}

class qopengl::WaveformRenderMark final : public QObject, public qopengl::WaveformRenderer {
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

    void generateMarkImage(WaveformMarkPointer pMark);
    void generatePlayPosMarkTexture();

    void drawTriangle(QPainter* painter,
            const QBrush& fillColor,
            QPointF p1,
            QPointF p2,
            QPointF p3);

    WaveformMarkSet m_marks;
    RGBAShader m_rgbaShader;
    TextureShader m_textureShader;
    std::unique_ptr<QOpenGLTexture> m_pPlayPosMarkTexture;
    bool m_bCuesUpdates{};

    void drawMark(const QRectF& rect, QColor color);
    void drawTexture(float x, float y, QOpenGLTexture* texture);

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderMark);
};
