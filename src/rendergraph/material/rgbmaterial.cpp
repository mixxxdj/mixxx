#include "rgbmaterial.h"

#include <QVector2D>

#include "rendergraph/materialshader.h"
#include "rendergraph/materialtype.h"
#include "rendergraph/uniformset.h"

using namespace rendergraph;

RGBMaterial::RGBMaterial()
        : Material(uniforms()) {
}

/* static */ const AttributeSet& RGBMaterial::attributes() {
    static AttributeSet set = makeAttributeSet<QVector2D, QVector3D>({"position", "color"});
    return set;
}

/* static */ const UniformSet& RGBMaterial::uniforms() {
    static UniformSet set = makeUniformSet<QMatrix4x4>({"ubuf.matrix"});
    return set;
}

MaterialType* RGBMaterial::type() const {
    static MaterialType type;
    return &type;
}

int RGBMaterial::compare(const Material* other) const {
    Q_ASSERT(other && type() == other->type());
    const auto* otherCasted = static_cast<const RGBMaterial*>(other);
    return otherCasted == this ? 0 : 1;
}

std::unique_ptr<MaterialShader> RGBMaterial::createShader() const {
    return std::make_unique<MaterialShader>(
            "rgb.vert", "rgb.frag", uniforms(), attributes());
}
