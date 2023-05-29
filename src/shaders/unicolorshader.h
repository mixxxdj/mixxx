#pragma once

#include "shaders/shader.h"

namespace mixxx {
class UnicolorShader;
}

class mixxx::UnicolorShader final : public mixxx::Shader {
  public:
    UnicolorShader() = default;
    ~UnicolorShader() = default;
    void init();

  private:
    DISALLOW_COPY_AND_ASSIGN(UnicolorShader)
};
