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
    void draw(const QMatrix4x4& matrix,
            float x,
            float y,
            const QString& s,
            QColor color,
            float devicePixelRatio);
    float height() const;

  private:
    mixxx::ColoredTextureShader m_shader;
    std::unique_ptr<QOpenGLTexture> m_pTexture;
    float m_offset[13];
    float m_width[12];

    DISALLOW_COPY_AND_ASSIGN(DigitsRenderer);
};
