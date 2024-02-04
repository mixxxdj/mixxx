#pragma once

#include "shaders/shader.h"

namespace mixxx {
class VinylQualityShader;
}

class mixxx::VinylQualityShader final : public mixxx::Shader {
  public:
    VinylQualityShader() = default;
    ~VinylQualityShader() = default;
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

    DISALLOW_COPY_AND_ASSIGN(VinylQualityShader)
};
