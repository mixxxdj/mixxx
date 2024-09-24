#include "rendergraph/assert.h"
#include "rendergraph/attributeset.h"

using namespace rendergraph;

BaseAttributeSet::BaseAttributeSet(std::initializer_list<AttributeInit> list,
        const std::vector<QString>& names) {
    DEBUG_ASSERT(list.size() == names.size());
    int i = 0;
    int offset = 0;
    m_attributes.reserve(list.size());
    for (auto item : list) {
        m_attributes.push_back(BaseGeometry::Attribute{
                offset, item.m_tupleSize, item.m_primitiveType, names[i++]});
        offset += item.m_tupleSize * sizeOf(item.m_primitiveType);
    }
    m_sizeOfVertex = offset;
}
