#pragma once

#include "backend/attributeset.h"

namespace rendergraph {
class AttributeSet;
}

class rendergraph::AttributeSet : public rendergraph::backend::AttributeSet {
  public:
    AttributeSet(std::initializer_list<Attribute> list, const std::vector<QString>& names);
    const std::vector<Attribute>& attributes() const;
};

namespace rendergraph {
template<typename... T>
AttributeSet makeAttributeSet(const std::vector<QString>& names) {
    return AttributeSet({(Attribute::create<T>())...}, names);
}
} // namespace rendergraph
