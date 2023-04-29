#pragma once

#include "qopengl/shaders/shader.h"

namespace qopengl {
class RGBShader;
}

class qopengl::RGBShader final : public qopengl::Shader {
  public:
    RGBShader() = default;
    ~RGBShader() = default;
    void init();

  private:
    DISALLOW_COPY_AND_ASSIGN(RGBShader)
};
