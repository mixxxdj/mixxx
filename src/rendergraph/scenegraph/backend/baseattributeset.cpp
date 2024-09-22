#include "backend/baseattributeset.h"

using namespace rendergraph;

namespace {
int toQSGGeometryType(const PrimitiveType& t) {
    switch (t) {
    case PrimitiveType::Float:
        return QSGGeometry::FloatType;
    case PrimitiveType::UInt:
        return QSGGeometry::UnsignedIntType;
    }
}
} // namespace

BaseAttributeSetHelper::BaseAttributeSetHelper(std::initializer_list<Attribute> list,
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
