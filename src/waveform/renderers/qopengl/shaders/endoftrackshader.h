#pragma once

#include "waveform/renderers/qopengl/shaders/shader.h"

namespace qopengl {
class EndOfTrackShader;
}

class qopengl::EndOfTrackShader : public qopengl::Shader {
  public:
    EndOfTrackShader() = default;
    ~EndOfTrackShader() = default;
    void init();

  private:
    DISALLOW_COPY_AND_ASSIGN(EndOfTrackShader)
};
