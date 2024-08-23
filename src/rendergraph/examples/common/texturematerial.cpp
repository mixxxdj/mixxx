#include "texturematerial.h"

#include <QMatrix4x4>
#include <QVector2D>

#include "rendergraph/materialshader.h"
#include "rendergraph/materialtype.h"

using namespace rendergraph;

TextureMaterial::TextureMaterial()
        : Material(uniforms()) {
}

/* static */ const AttributeSet& TextureMaterial::attributes() {
    static AttributeSet set = makeAttributeSet<QVector2D, QVector2D>({"position", "texcoord"});
    return set;
}

/* static */ const UniformSet& TextureMaterial::uniforms() {
    static UniformSet set = makeUniformSet<QMatrix4x4>({"ubuf.matrix"});
    return set;
}

MaterialType* TextureMaterial::type() const {
    static MaterialType type;
    return &type;
}

int TextureMaterial::compare(const Material* other) const {
    Q_ASSERT(other && type() == other->type());
    const auto* otherCasted = static_cast<const TextureMaterial*>(other);
    return otherCasted == this ? 0 : 1;
}

MaterialShader* TextureMaterial::createShader() const {
    return new MaterialShader("texture.vert", "texture.frag", uniforms(), attributes());
}
