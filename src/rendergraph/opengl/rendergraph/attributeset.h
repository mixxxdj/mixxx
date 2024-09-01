#pragma once

#include <initializer_list>
#include <vector>

#include "rendergraph/attribute.h"

namespace rendergraph {
class AttributeSet;
}

class rendergraph::AttributeSet {
  public:
    AttributeSet();
    AttributeSet(std::initializer_list<Attribute> list, const std::vector<QString>& names);

    ~AttributeSet();

    const std::vector<Attribute>& attributes() const;

  private:
    void add(const Attribute& attribute);

    std::vector<Attribute> m_attributes;
};

namespace rendergraph {
template<typename... T>
AttributeSet makeAttributeSet(const std::vector<QString>& names) {
    return AttributeSet({(Attribute::create<T>())...}, names);
}
} // namespace rendergraph
