#pragma once

#include "rendergraph/attributeset.h"

class rendergraph::AttributeSet::Impl {
  public:
    void add(const Attribute& attribute) {
        m_attributes.push_back(attribute);
    }

    const std::vector<Attribute>& attributes() const {
        return m_attributes;
    }

  private:
    std::vector<Attribute> m_attributes;
};
