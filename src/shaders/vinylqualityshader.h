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
    int samplerLocation() const {
        return m_samplerLocation;
    }
    int colorLocation() const {
        return m_colorLocation;
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
    int m_colorLocation;
    int m_positionLocation;
    int m_texcoordLocation;

    DISALLOW_COPY_AND_ASSIGN(VinylQualityShader)
};
