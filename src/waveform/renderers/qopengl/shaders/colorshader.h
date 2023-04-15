#include "waveform/renderers/qopengl/shaders/shader.h"

namespace qopengl {
class ColorShader;
}

class qopengl::ColorShader final : public qopengl::Shader {
  public:
    ColorShader() = default;
    ~ColorShader() = default;
    void init();

  private:
    DISALLOW_COPY_AND_ASSIGN(ColorShader)
};
