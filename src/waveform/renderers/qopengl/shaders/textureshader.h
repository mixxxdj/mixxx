#include "waveform/renderers/qopengl/shaders/shader.h"

namespace qopengl {
class TextureShader;
}

class qopengl::TextureShader final : public qopengl::Shader {
  public:
    TextureShader() = default;
    ~TextureShader() = default;
    void init();

  private:
    DISALLOW_COPY_AND_ASSIGN(TextureShader)
};
