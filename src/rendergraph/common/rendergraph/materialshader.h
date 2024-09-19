#pragma once

#include "backend/materialshader.h"
#include "rendergraph/attributeset.h"
#include "rendergraph/uniformset.h"

namespace rendergraph {
class MaterialShader;
} // namespace rendergraph

class rendergraph::MaterialShader : public rendergraph::backend::MaterialShader {
  public:
    MaterialShader(const char* vertexShaderFile,
            const char* fragmentShaderFile,
            const UniformSet& uniforms,
            const AttributeSet& attributeSet);
};
