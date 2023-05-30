#pragma once

#include "shaders/shader.h"

namespace mixxx {
class RGBShader;
}

class mixxx::RGBShader final : public mixxx::Shader {
  public:
    RGBShader() = default;
    ~RGBShader() = default;
    void init();

  private:
    DISALLOW_COPY_AND_ASSIGN(RGBShader)
};
