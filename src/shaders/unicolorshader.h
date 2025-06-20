#pragma once

#include "shaders/shader.h"

namespace mixxx {
class UnicolorShader;
}

class mixxx::UnicolorShader final : public mixxx::Shader {
  public:
    UnicolorShader() = default;
    ~UnicolorShader() = default;
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

    DISALLOW_COPY_AND_ASSIGN(UnicolorShader)
};
