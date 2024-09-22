#include "rendergraph/attributeset.h"
#include "rendergraph/material.h"

namespace rendergraph {
class RGBMaterial;
}

class rendergraph::RGBMaterial : public rendergraph::Material {
  public:
    RGBMaterial();

    static const AttributeSet& attributes();

    static const UniformSet& uniforms();

    MaterialType* type() const override;

    std::unique_ptr<MaterialShader> createShader() const override;
};
