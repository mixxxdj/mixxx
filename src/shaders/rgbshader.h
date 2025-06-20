#pragma once

#include "shaders/shader.h"

namespace mixxx {
class RGBShader;
}

class mixxx::RGBShader final : public mixxx::Shader {
  public:
    RGBShader() = default;
    ~RGBShader() = default;
    void init();

    int matrixLocation() const {
        return m_matrixLocation;
    }
    int positionLocation() const {
        return m_positionLocation;
    }
    int colorLocation() const {
        return m_colorLocation;
    }

  private:
    int m_matrixLocation;
    int m_positionLocation;
    int m_colorLocation;

    DISALLOW_COPY_AND_ASSIGN(RGBShader)
};
