#pragma once

#include "waveform/renderers/qopengl/shaders/shader.h"

namespace qopengl {
class UnicolorShader;
}

class qopengl::UnicolorShader : public qopengl::Shader {
  public:
    void init();
};
