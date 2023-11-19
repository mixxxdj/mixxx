#pragma once

#include "shaders/shader.h"

namespace mixxx {
class EndOfTrackShader;
}

class mixxx::EndOfTrackShader : public mixxx::Shader {
  public:
    EndOfTrackShader() = default;
    ~EndOfTrackShader() = default;
    void init();

    int positionLocation() const {
        return m_positionLocation;
    }
    int gradientLocation() const {
        return m_gradientLocation;
    }
    int colorLocation() const {
        return m_colorLocation;
    }

  private:
    int m_positionLocation;
    int m_gradientLocation;
    int m_colorLocation;

    DISALLOW_COPY_AND_ASSIGN(EndOfTrackShader)
};
