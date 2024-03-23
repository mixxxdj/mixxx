#pragma once

#include "shaders/shader.h"

namespace mixxx {
class ColoredTextureShader;
}

class mixxx::ColoredTextureShader final : public mixxx::Shader {
  public:
    ColoredTextureShader() = default;
    ~ColoredTextureShader() = default;
    void init();

    int matrixLocation() const {
        return m_matrixLocation;
    }
    int positionLocation() const {
        return m_positionLocation;
    }
    int texcoordLocation() const {
        return m_texcoordLocation;
    }
    int textureLocation() const {
        return m_textureLocation;
    }
    int colorLocation() const {
        return m_colorLocation;
    }

  private:
    int m_matrixLocation;
    int m_positionLocation;
    int m_texcoordLocation;
    int m_textureLocation;
    int m_colorLocation;

    DISALLOW_COPY_AND_ASSIGN(ColoredTextureShader)
};
