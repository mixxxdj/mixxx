#include "waveform/renderers/qopengl/shaders/shader.h"

namespace qopengl {
class GradientShader;
}

class qopengl::GradientShader : public qopengl::Shader {
  public:
    void init();
};
