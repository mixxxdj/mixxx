#pragma once

#include "shaders/shader.h"

namespace mixxx {
class PatternShader;
}

class mixxx::PatternShader final : public mixxx::Shader {
  public:
    PatternShader() = default;
    ~PatternShader() = default;
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
    int repetitionsLocation() const {
        return m_repetitionsLocation;
    }

  private:
    int m_matrixLocation;
    int m_positionLocation;
    int m_texcoordLocation;
    int m_textureLocation;
    int m_repetitionsLocation;

    DISALLOW_COPY_AND_ASSIGN(PatternShader)
};
