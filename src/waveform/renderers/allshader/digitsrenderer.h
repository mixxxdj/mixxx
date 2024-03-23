#pragma once

#include <QOpenGLFunctions>

#include "shaders/coloredtextureshader.h"

class QOpenGLTexture;

namespace allshader {
class DigitsRenderer;
}

class allshader::DigitsRenderer : public QOpenGLFunctions {
  public:
    DigitsRenderer() = default;
    ~DigitsRenderer();

    void init();
    void generateTexture(float devicePixelRatio);
    void drawNumber(const QMatrix4x4& matrix,
            float x,
            float y,
            int number,
            QColor color,
            float devicePixelRatio);
    float height() const;

  private:
    mixxx::ColoredTextureShader m_shader;
    std::unique_ptr<QOpenGLTexture> m_pTexture;
    qreal m_offset[10];
    qreal m_width[10];

    DISALLOW_COPY_AND_ASSIGN(DigitsRenderer);
};
