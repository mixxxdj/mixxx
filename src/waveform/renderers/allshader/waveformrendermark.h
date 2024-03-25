#pragma once

#include <QColor>

#include "shaders/rgbashader.h"
#include "shaders/textureshader.h"
#include "waveform/renderers/allshader/digitsrenderer.h"
#include "waveform/renderers/allshader/waveformrendererabstract.h"
#include "waveform/renderers/waveformrendermarkbase.h"

class QDomNode;
class SkinContext;
class QOpenGLTexture;

namespace allshader {
class WaveformRenderMark;
}

class allshader::WaveformRenderMark : public ::WaveformRenderMarkBase,
                                      public allshader::WaveformRendererAbstract {
  public:
    explicit WaveformRenderMark(WaveformWidgetRenderer* waveformWidget);

    void draw(QPainter* painter, QPaintEvent* event) override {
        Q_UNUSED(painter);
        Q_UNUSED(event);
    }

    allshader::WaveformRendererAbstract* allshaderWaveformRenderer() override {
        return this;
    }

    bool init() override;

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

  private:
    void updateMarkImage(WaveformMarkPointer pMark) override;

    void updatePlayPosMarkTexture();

    void drawTriangle(QPainter* painter,
            const QBrush& fillColor,
            QPointF p1,
            QPointF p2,
            QPointF p3);

    void drawMark(const QMatrix4x4& matrix, const QRectF& rect, QColor color);
    void drawTexture(const QMatrix4x4& matrix, float x, float y, QOpenGLTexture* texture);
    void updateUntilMark(double playPosition, double markerPosition);

    mixxx::RGBAShader m_rgbaShader;
    mixxx::TextureShader m_textureShader;
    std::unique_ptr<QOpenGLTexture> m_pPlayPosMarkTexture;
    DigitsRenderer m_digitsRenderer;
    int m_beatsUntilMark;
    double m_timeUntilMark;
    double m_currentBeatPosition;
    double m_nextBeatPosition;
    std::unique_ptr<ControlProxy> m_pTimeRemainingControl;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderMark);
};
