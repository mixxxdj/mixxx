#include "waveform/renderers/qopengl/shaders/shader.h"

namespace qopengl {
class EndOfTrackShader;
}

class qopengl::EndOfTrackShader : public qopengl::Shader {
  public:
    void init();
};
