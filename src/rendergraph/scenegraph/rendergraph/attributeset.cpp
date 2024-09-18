#include "rendergraph/attributeset.h"

using namespace rendergraph;

AttributeSetBase::AttributeSetBase(std::initializer_list<Attribute> list,
        const std::vector<QString>& names) {
    int i = 0;
    for (auto item : list) {
        add(Attribute{item.m_tupleSize, item.m_primitiveType, names[i++]});
    }
}

void AttributeSetBase::add(const Attribute& attribute) {
    m_attributes.push_back(attribute);

    const int count = static_cast<int>(m_sgAttributes.size());
    const bool isPosition = count == 0;
    m_sgAttributes.push_back(QSGGeometry::Attribute::create(count,
            attribute.m_tupleSize,
            toQSGGeometryType(attribute.m_primitiveType),
            isPosition));
    m_stride += attribute.m_tupleSize * sizeOf(attribute.m_primitiveType);
}

int AttributeSetBase::toQSGGeometryType(const rendergraph::PrimitiveType& t) {
    switch (t) {
    case rendergraph::PrimitiveType::Float:
        return QSGGeometry::FloatType;
    case rendergraph::PrimitiveType::UInt:
        return QSGGeometry::UnsignedIntType;
    }
}

AttributeSet::AttributeSet(std::initializer_list<Attribute> list,
        const std::vector<QString>& names)
        : AttributeSetBase(list, names),
          QSGGeometry::AttributeSet{static_cast<int>(m_sgAttributes.size()),
                  m_stride,
                  m_sgAttributes.data()} {
}

AttributeSet::~AttributeSet() = default;
