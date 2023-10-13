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
    int samplerLocation() const {
        return m_samplerLocation;
    }
    int repetitionsLocation() const {
        return m_repetitionsLocation;
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
    int m_repetitionsLocation;
    int m_texcoordLocation;
    int m_positionLocation;

    DISALLOW_COPY_AND_ASSIGN(PatternShader)
};
