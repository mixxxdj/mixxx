#pragma once

#include "qopengl/shaders/shader.h"

namespace qopengl {
class UnicolorShader;
}

class qopengl::UnicolorShader final : public qopengl::Shader {
  public:
    UnicolorShader() = default;
    ~UnicolorShader() = default;
    void init();

  private:
    DISALLOW_COPY_AND_ASSIGN(UnicolorShader)
};
