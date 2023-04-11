#include "waveform/renderers/qopengl/shaders/shader.h"

namespace qopengl {
class TextureShader;
}

class qopengl::TextureShader : public qopengl::Shader {
  public:
    void init();
};
