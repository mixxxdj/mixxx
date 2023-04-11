#include "waveform/renderers/qopengl/shaders/shader.h"

namespace qopengl {
class ColorShader;
}

class qopengl::ColorShader : public qopengl::Shader {
  public:
    void init();
};
