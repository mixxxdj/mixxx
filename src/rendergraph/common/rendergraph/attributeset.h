#pragma once

#include "backend/baseattributeset.h"
#include "rendergraph/attributeinit.h"

namespace rendergraph {
class AttributeSet;
}

class rendergraph::AttributeSet : public rendergraph::BaseAttributeSet {
  public:
    AttributeSet(std::initializer_list<AttributeInit> list, const std::vector<QString>& names);
};

namespace rendergraph {
template<typename... T>
AttributeSet makeAttributeSet(const std::vector<QString>& names) {
    return AttributeSet({(AttributeInit::create<T>())...}, names);
}
} // namespace rendergraph
