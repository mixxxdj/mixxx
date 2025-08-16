#include "rgbamaterial.h"

#include <QVector2D>

#include "rendergraph/materialshader.h"
#include "rendergraph/materialtype.h"
#include "rendergraph/uniformset.h"

using namespace rendergraph;

RGBAMaterial::RGBAMaterial()
        : Material(uniforms()) {
}

// static
const AttributeSet& RGBAMaterial::attributes() {
    static AttributeSet set = makeAttributeSet<QVector2D, QVector4D>({"position", "color"});
    return set;
}

// static
const UniformSet& RGBAMaterial::uniforms() {
    static UniformSet set = makeUniformSet<QMatrix4x4>({"ubuf.matrix"});
    return set;
}

MaterialType* RGBAMaterial::type() const {
    static MaterialType type;
    return &type;
}

std::unique_ptr<MaterialShader> RGBAMaterial::createShader() const {
    return std::make_unique<MaterialShader>(
            "rgba.vert", "rgba.frag", uniforms(), attributes());
}
