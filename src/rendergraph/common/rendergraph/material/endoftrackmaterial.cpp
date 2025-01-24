#include "endoftrackmaterial.h"

#include <QVector2D>

#include "rendergraph/materialshader.h"
#include "rendergraph/materialtype.h"
#include "rendergraph/uniformset.h"

using namespace rendergraph;

EndOfTrackMaterial::EndOfTrackMaterial()
        : Material(uniforms()) {
}

/* static */ const AttributeSet& EndOfTrackMaterial::attributes() {
    static AttributeSet set = makeAttributeSet<QVector2D, float>({"position", "gradient"});
    return set;
}

/* static */ const UniformSet& EndOfTrackMaterial::uniforms() {
    static UniformSet set = makeUniformSet<QVector4D>({"ubuf.color"});
    return set;
}

MaterialType* EndOfTrackMaterial::type() const {
    static MaterialType type;
    return &type;
}

std::unique_ptr<MaterialShader> EndOfTrackMaterial::createShader() const {
    return std::make_unique<MaterialShader>(
            "endoftrack.vert", "endoftrack.frag", uniforms(), attributes());
}
