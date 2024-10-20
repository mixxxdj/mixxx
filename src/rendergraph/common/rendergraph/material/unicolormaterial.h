#include "rendergraph/attributeset.h"
#include "rendergraph/material.h"

namespace rendergraph {
class UniColorMaterial;
}

class rendergraph::UniColorMaterial : public rendergraph::Material {
  public:
    UniColorMaterial();

    static const AttributeSet& attributes();

    static const UniformSet& uniforms();

    MaterialType* type() const override;

    std::unique_ptr<MaterialShader> createShader() const override;
};
