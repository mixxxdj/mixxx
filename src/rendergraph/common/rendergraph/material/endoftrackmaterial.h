#include "rendergraph/attributeset.h"
#include "rendergraph/material.h"

namespace rendergraph {
class EndOfTrackMaterial;
}

class rendergraph::EndOfTrackMaterial : public rendergraph::Material {
  public:
    EndOfTrackMaterial();

    static const AttributeSet& attributes();

    static const UniformSet& uniforms();

    MaterialType* type() const override;

    std::unique_ptr<MaterialShader> createShader() const override;
};
