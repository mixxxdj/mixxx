#pragma once

#include "backend/baseattributeset.h"

namespace rendergraph {
class AttributeSet;
}

class rendergraph::AttributeSet : public rendergraph::BaseAttributeSet {
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
