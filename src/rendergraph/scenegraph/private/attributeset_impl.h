#pragma once

#include <QSGGeometry>

#include "rendergraph/attributeset.h"

class rendergraph::AttributeSet::Impl {
  public:
    void add(const Attribute& attribute) {
        const int count = static_cast<int>(m_sgAttributes.size());
        const bool isPosition = count == 0;
        m_sgAttributes.push_back(QSGGeometry::Attribute::create(count,
                attribute.m_tupleSize,
                toQSGGeometryType(attribute.m_primitiveType),
                isPosition));
        const int stride = m_sgAttributeSet.stride +
                attribute.m_tupleSize * sizeOf(attribute.m_primitiveType);
        m_sgAttributeSet = QSGGeometry::AttributeSet{count + 1, stride, m_sgAttributes.data()};
    }

    const QSGGeometry::AttributeSet& sgAttributeSet() const {
        return m_sgAttributeSet;
    }

  private:
    QSGGeometry::AttributeSet m_sgAttributeSet{};
    std::vector<QSGGeometry::Attribute> m_sgAttributes;

    int toQSGGeometryType(const rendergraph::PrimitiveType& t) {
        switch (t) {
        case rendergraph::PrimitiveType::Float:
            return QSGGeometry::FloatType;
        case rendergraph::PrimitiveType::UInt:
            return QSGGeometry::UnsignedIntType;
        }
    }
};
