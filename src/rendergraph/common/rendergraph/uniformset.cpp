#include "rendergraph/uniformset.h"

using namespace rendergraph;

UniformSet::UniformSet(std::initializer_list<Uniform> list, const std::vector<QString>& names) {
    for (std::size_t i = 0; i < list.size() && i < names.size(); ++i) {
        add(Uniform{list.begin()[i].m_type, names[i]});
    }
}

UniformSet::~UniformSet() = default;

void UniformSet::add(const Uniform& uniform) {
    m_uniforms.push_back(uniform);
}

const std::vector<Uniform>& UniformSet::uniforms() const {
    return m_uniforms;
}
