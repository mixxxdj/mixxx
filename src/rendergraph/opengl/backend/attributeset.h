#pragma once

#include <initializer_list>
#include <vector>

#include "rendergraph/attribute.h"

namespace rendergraph::backend {
class AttributeSet;
}

class rendergraph::backend::AttributeSet {
  protected:
    AttributeSet(std::initializer_list<Attribute> list, const std::vector<QString>& names);
    std::vector<Attribute> m_attributes;
};
