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

  private:
    DISALLOW_COPY_AND_ASSIGN(EndOfTrackShader)
};
