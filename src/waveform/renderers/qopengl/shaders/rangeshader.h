#include "waveform/renderers/qopengl/shaders/shader.h"

namespace qopengl {
class RangeShader;
}

class qopengl::RangeShader : public qopengl::Shader {
  public:
    void init();
};
