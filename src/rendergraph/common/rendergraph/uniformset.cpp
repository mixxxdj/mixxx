#include "rendergraph/uniformset.h"

using namespace rendergraph;

UniformSet::UniformSet(std::initializer_list<Uniform> list, const std::vector<QString>& names) {
    int i = 0;
    for (auto item : list) {
        add(Uniform{item.m_type, names[i++]});
    }
}

UniformSet::~UniformSet() = default;

void UniformSet::add(const Uniform& uniform) {
    m_uniforms.push_back(uniform);
}

const std::vector<Uniform>& UniformSet::uniforms() const {
    return m_uniforms;
}
