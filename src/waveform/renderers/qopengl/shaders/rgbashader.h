#pragma once

#include "waveform/renderers/qopengl/shaders/shader.h"

namespace qopengl {
class RGBAShader;
}

class qopengl::RGBAShader final : public qopengl::Shader {
  public:
    RGBAShader() = default;
    ~RGBAShader() = default;
    void init();

  private:
    DISALLOW_COPY_AND_ASSIGN(RGBAShader)
};
