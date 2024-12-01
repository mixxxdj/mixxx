#include "rendergraph/attributeset.h"
#include "rendergraph/material.h"

namespace rendergraph {
class RGBAMaterial;
}

class rendergraph::RGBAMaterial : public rendergraph::Material {
  public:
    RGBAMaterial();

    static const AttributeSet& attributes();

    static const UniformSet& uniforms();

    MaterialType* type() const override;

    std::unique_ptr<MaterialShader> createShader() const override;
};
