#pragma once

#include <initializer_list>
#include <vector>

#include "rendergraph/attribute.h"

namespace rendergraph {
class BaseAttributeSet;
}

class rendergraph::BaseAttributeSet {
  protected:
    BaseAttributeSet(std::initializer_list<Attribute> list, const std::vector<QString>& names);
    std::vector<Attribute> m_attributes;
};
