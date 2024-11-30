#pragma once

#include "backend/baseattributeset.h"
#include "rendergraph/attributeinit.h"

namespace rendergraph {

class AttributeSet : public BaseAttributeSet {
  public:
    AttributeSet(std::initializer_list<AttributeInit> list, const std::vector<QString>& names);
};

template<typename... Ts, int N>
AttributeSet makeAttributeSet(const QString (&names)[N]) {
    static_assert(sizeof...(Ts) == N, "Mismatch between number of attribute types and names");
    return AttributeSet({(AttributeInit::create<Ts>())...},
            std::vector<QString>(std::cbegin(names), std::cend(names)));
}

} // namespace rendergraph
