#include "patternmaterial.h"

#include <QMatrix4x4>
#include <QVector2D>

#include "rendergraph/materialshader.h"
#include "rendergraph/materialtype.h"

using namespace rendergraph;

PatternMaterial::PatternMaterial()
        : Material(uniforms()) {
}

/* static */ const AttributeSet& PatternMaterial::attributes() {
    static AttributeSet set = makeAttributeSet<QVector2D, QVector2D>({"position", "texcoord"});
    return set;
}

/* static */ const UniformSet& PatternMaterial::uniforms() {
    static UniformSet set = makeUniformSet<QMatrix4x4>({"ubuf.matrix"});
    return set;
}

MaterialType* PatternMaterial::type() const {
    static MaterialType type;
    return &type;
}

int PatternMaterial::compare(const Material* other) const {
    Q_ASSERT(other && type() == other->type());
    const auto* otherCasted = static_cast<const PatternMaterial*>(other);
    return otherCasted == this ? 0 : 1;
}

std::unique_ptr<MaterialShader> PatternMaterial::createShader() const {
    return std::make_unique<MaterialShader>(
            "pattern.vert", "pattern.frag", uniforms(), attributes());
}
