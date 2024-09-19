#pragma once

#include <QSGGeometry>
#include <initializer_list>
#include <vector>

#include "rendergraph/attribute.h"

namespace rendergraph::backend {
class AttributeSet;
class AttributeSetBase;
} // namespace rendergraph::backend

class rendergraph::backend::AttributeSetBase {
  protected:
    int m_stride{};

    AttributeSetBase(std::initializer_list<Attribute> list, const std::vector<QString>& names);

    const std::vector<Attribute>& attributes() const {
        return m_attributes;
    }

    std::vector<Attribute> m_attributes;
    QSGGeometry::AttributeSet m_sgAttributeSet{};
    std::vector<QSGGeometry::Attribute> m_sgAttributes;
};

class rendergraph::backend::AttributeSet
        : protected rendergraph::backend::AttributeSetBase,
          public QSGGeometry::AttributeSet {
  protected:
    AttributeSet(std::initializer_list<Attribute> list,
            const std::vector<QString>& names)
            : AttributeSetBase(list, names),
              QSGGeometry::AttributeSet{static_cast<int>(m_sgAttributes.size()),
                      m_stride,
                      m_sgAttributes.data()} {
    }
};
