#include "rendergraph/attributeset.h"

using namespace rendergraph;

AttributeSet::AttributeSet() = default;

AttributeSet::AttributeSet(std::initializer_list<Attribute> list, const std::vector<QString>& names)
        : AttributeSet() {
    int i = 0;
    for (auto item : list) {
        add(Attribute{item.m_tupleSize, item.m_primitiveType, names[i++]});
    }
}

AttributeSet::~AttributeSet() = default;

void AttributeSet::add(const Attribute& attribute) {
    m_attributes.push_back(attribute);
}

const std::vector<Attribute>& AttributeSet::attributes() const {
    return m_attributes;
}
