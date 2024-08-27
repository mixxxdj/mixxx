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

    int compare(const Material* other) const override;

    MaterialShader* createShader() const override;
};
