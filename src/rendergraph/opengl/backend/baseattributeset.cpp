#include "rendergraph/attributeset.h"

using namespace rendergraph;

BaseAttributeSet::BaseAttributeSet(std::initializer_list<Attribute> list,
        const std::vector<QString>& names) {
    int i = 0;
    for (auto item : list) {
        m_attributes.push_back(Attribute{item.m_tupleSize, item.m_primitiveType, names[i++]});
    }
}
