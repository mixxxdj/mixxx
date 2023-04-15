#include "waveform/renderers/qopengl/shaders/shader.h"

namespace qopengl {
class GradientShader;
}

class qopengl::GradientShader : public qopengl::Shader {
  public:
    GradientShader() = default;
    ~GradientShader() = default;
    void init();

  private:
    DISALLOW_COPY_AND_ASSIGN(GradientShader)
};
