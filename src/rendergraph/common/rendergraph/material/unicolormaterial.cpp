#include "unicolormaterial.h"

#include <QVector2D>

#include "rendergraph/materialshader.h"
#include "rendergraph/materialtype.h"
#include "rendergraph/uniformset.h"

using namespace rendergraph;

UniColorMaterial::UniColorMaterial()
        : Material(uniforms()) {
}

/* static */ const AttributeSet& UniColorMaterial::attributes() {
    static AttributeSet set = makeAttributeSet<QVector2D>({"position"});
    return set;
}

/* static */ const UniformSet& UniColorMaterial::uniforms() {
    static UniformSet set = makeUniformSet<QMatrix4x4, QVector4D>({"ubuf.matrix", "ubuf.color"});
    return set;
}

MaterialType* UniColorMaterial::type() const {
    static MaterialType type;
    return &type;
}

std::unique_ptr<MaterialShader> UniColorMaterial::createShader() const {
    return std::make_unique<MaterialShader>(
            "unicolor.vert", "unicolor.frag", uniforms(), attributes());
}
