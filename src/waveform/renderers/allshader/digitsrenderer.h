#pragma once

#include <QOpenGLFunctions>

#include "shaders/textureshader.h"
#include "util/opengltexture2d.h"

namespace allshader {
class DigitsRenderer;
}

class allshader::DigitsRenderer : public QOpenGLFunctions {
  public:
    DigitsRenderer() = default;
    ~DigitsRenderer();

    void init();
    void generateTexture(int fontPixelSize, float devicePixelRatio);
    float draw(const QMatrix4x4& matrix,
            float x,
            float y,
            const QString& s,
            float devicePixelRatio);
    float height() const;

  private:
    mixxx::TextureShader m_shader;
    OpenGLTexture2D m_texture;
    float m_offset[13];
    float m_width[12];

    DISALLOW_COPY_AND_ASSIGN(DigitsRenderer);
};
