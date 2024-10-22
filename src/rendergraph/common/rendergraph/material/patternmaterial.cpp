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

std::unique_ptr<MaterialShader> PatternMaterial::createShader() const {
    return std::make_unique<MaterialShader>(
            "pattern.vert", "pattern.frag", uniforms(), attributes());
}
