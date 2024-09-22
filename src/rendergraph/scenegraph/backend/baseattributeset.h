#pragma once

#include <QSGGeometry>
#include <initializer_list>
#include <vector>

#include "rendergraph/attribute.h"

namespace rendergraph {
class BaseAttributeSet;
class BaseAttributeSetHelper;
} // namespace rendergraph

class rendergraph::BaseAttributeSetHelper {
  protected:
    // helper base class for BaseAttributeSet to populate m_attributes, and m_sgAttributes
    // needed to construct BaseAttributeSet's other base class, QSGGeometry::AttributeSet
    int m_stride{};

    BaseAttributeSetHelper(std::initializer_list<Attribute> list,
            const std::vector<QString>& names);

    const std::vector<Attribute>& attributes() const {
        return m_attributes;
    }

    std::vector<Attribute> m_attributes;
    std::vector<QSGGeometry::Attribute> m_sgAttributes;
};

class rendergraph::BaseAttributeSet
        : protected rendergraph::BaseAttributeSetHelper,
          public QSGGeometry::AttributeSet {
  protected:
    BaseAttributeSet(std::initializer_list<Attribute> list,
            const std::vector<QString>& names)
            : BaseAttributeSetHelper(list, names),
              QSGGeometry::AttributeSet{static_cast<int>(m_sgAttributes.size()),
                      m_stride,
                      m_sgAttributes.data()} {
    }
};
