#pragma once

#include <initializer_list>
#include <vector>

#include "backend/basegeometry.h"
#include "rendergraph/attributeinit.h"

namespace rendergraph {
class BaseAttributeSet;
}

class rendergraph::BaseAttributeSet {
  protected:
    BaseAttributeSet(std::initializer_list<AttributeInit> list, const std::vector<QString>& names);

  public:
    const std::vector<BaseGeometry::Attribute>& attributes() const {
        return m_attributes;
    }
    int sizeOfVertex() const {
        return m_sizeOfVertex;
    }

  protected:
    std::vector<BaseGeometry::Attribute> m_attributes;
    int m_sizeOfVertex;
};
