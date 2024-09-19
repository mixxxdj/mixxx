#include "backend/attributeset.h"

using namespace rendergraph::backend;

namespace {
int toQSGGeometryType(const rendergraph::PrimitiveType& t) {
    switch (t) {
    case rendergraph::PrimitiveType::Float:
        return QSGGeometry::FloatType;
    case rendergraph::PrimitiveType::UInt:
        return QSGGeometry::UnsignedIntType;
    }
}
} // namespace

AttributeSetBase::AttributeSetBase(std::initializer_list<Attribute> list,
        const std::vector<QString>& names) {
    int i = 0;
    for (auto item : list) {
        m_attributes.push_back(Attribute{item.m_tupleSize, item.m_primitiveType, names[i++]});

        const auto& attribute = m_attributes.back();

        const int count = static_cast<int>(m_sgAttributes.size());
        const bool isPosition = count == 0;
        m_sgAttributes.push_back(QSGGeometry::Attribute::create(count,
                attribute.m_tupleSize,
                toQSGGeometryType(attribute.m_primitiveType),
                isPosition));
        m_stride += attribute.m_tupleSize * sizeOf(attribute.m_primitiveType);
    }
}
