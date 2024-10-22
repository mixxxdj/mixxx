#include "backend/baseattributeset.h"

#include <stdexcept>

using namespace rendergraph;

namespace {
int toQSGGeometryType(const PrimitiveType& t) {
    switch (t) {
    case PrimitiveType::Float:
        return QSGGeometry::FloatType;
    case PrimitiveType::UInt:
        return QSGGeometry::UnsignedIntType;
    default:
        throw std::runtime_error("not implemented");
    }
}
} // namespace

BaseAttributeSetHelper::BaseAttributeSetHelper(std::initializer_list<AttributeInit> list) {
    int i = 0;
    m_sgAttributes.reserve(list.size());
    for (auto item : list) {
        const int count = static_cast<int>(m_sgAttributes.size());
        const bool isPosition = count == 0;
        m_sgAttributes.push_back(QSGGeometry::Attribute::create(count,
                item.m_tupleSize,
                toQSGGeometryType(item.m_primitiveType),
                isPosition));
    }
}
