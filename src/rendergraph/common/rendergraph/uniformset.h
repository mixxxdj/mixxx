#pragma once

#include <QString>
#include <initializer_list>
#include <vector>

#include "rendergraph/uniform.h"

namespace rendergraph {
class UniformSet;
}

class rendergraph::UniformSet {
  public:
    UniformSet(std::initializer_list<Uniform> list, const std::vector<QString>& names);

    ~UniformSet();

    const std::vector<Uniform>& uniforms() const;

  private:
    void add(const Uniform& uniform);
    std::vector<Uniform> m_uniforms;
};

namespace rendergraph {
template<typename... T>
UniformSet makeUniformSet(const std::vector<QString>& names) {
    return UniformSet({(Uniform::create<T>())...}, names);
}
} // namespace rendergraph
