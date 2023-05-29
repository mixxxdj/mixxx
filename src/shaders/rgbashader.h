#pragma once

#include "shaders/shader.h"

namespace mixxx {
class RGBAShader;
}

class mixxx::RGBAShader final : public mixxx::Shader {
  public:
    RGBAShader() = default;
    ~RGBAShader() = default;
    void init();

  private:
    DISALLOW_COPY_AND_ASSIGN(RGBAShader)
};
