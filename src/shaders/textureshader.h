#pragma once

#include "shaders/shader.h"

namespace mixxx {
class TextureShader;
}

class mixxx::TextureShader final : public mixxx::Shader {
  public:
    TextureShader() = default;
    ~TextureShader() = default;
    void init();

    int matrixLocation() const {
        return m_matrixLocation;
    }
    int samplerLocation() const {
        return m_samplerLocation;
    }
    int positionLocation() const {
        return m_positionLocation;
    }
    int texcoordLocation() const {
        return m_texcoordLocation;
    }

  private:
    int m_matrixLocation;
    int m_samplerLocation;
    int m_positionLocation;
    int m_texcoordLocation;

    DISALLOW_COPY_AND_ASSIGN(TextureShader)
};
