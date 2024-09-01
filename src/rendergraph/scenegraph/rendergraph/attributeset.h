#pragma once

#include <initializer_list>
#include <vector>

#include "rendergraph/attribute.h"

namespace rendergraph {
class AttributeSet;
}

class rendergraph::AttributeSet {
  public:
    class Impl;

    AttributeSet();
    AttributeSet(std::initializer_list<Attribute> list, const std::vector<QString>& names);

    ~AttributeSet();

    const std::vector<Attribute>& attributes() const;
    Impl& impl() const;

  private:
    AttributeSet(Impl* pImpl);
    void add(const Attribute& attribute);

    std::vector<Attribute> m_attributes;
    const std::unique_ptr<Impl> m_pImpl;
};

namespace rendergraph {
template<typename... T>
AttributeSet makeAttributeSet(const std::vector<QString>& names) {
    return AttributeSet({(Attribute::create<T>())...}, names);
}
} // namespace rendergraph
