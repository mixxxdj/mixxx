#pragma once

#include "shaders/shader.h"

namespace mixxx {
class SlipModeShader;
}

class mixxx::SlipModeShader : public mixxx::Shader {
  public:
    SlipModeShader() = default;
    ~SlipModeShader() = default;
    void init();

    int positionLocation() const {
        return m_positionLocation;
    }
    int dimensionLocation() const {
        return m_dimensionLocation;
    }
    int colorLocation() const {
        return m_colorLocation;
    }
    int boarderLocation() const {
        return m_boarderLocation;
    }

  private:
    int m_positionLocation;
    int m_dimensionLocation;
    int m_colorLocation;
    int m_boarderLocation;

    DISALLOW_COPY_AND_ASSIGN(SlipModeShader)
};
