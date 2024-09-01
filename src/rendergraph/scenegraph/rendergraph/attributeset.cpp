#include "rendergraph/attributeset.h"

#include "attributeset_impl.h"

using namespace rendergraph;

AttributeSet::AttributeSet(AttributeSet::Impl* pImpl)
        : m_pImpl(pImpl) {
}

AttributeSet::AttributeSet()
        : AttributeSet(new AttributeSet::Impl()) {
}

AttributeSet::AttributeSet(std::initializer_list<Attribute> list, const std::vector<QString>& names)
        : AttributeSet() {
    int i = 0;
    for (auto item : list) {
        add(Attribute{item.m_tupleSize, item.m_primitiveType, names[i++]});
    }
}

AttributeSet::~AttributeSet() = default;

AttributeSet::Impl& AttributeSet::impl() const {
    return *m_pImpl;
}

void AttributeSet::add(const Attribute& attribute) {
    m_attributes.push_back(attribute);
    m_pImpl->add(attribute);
}

const std::vector<Attribute>& AttributeSet::attributes() const {
    return m_attributes;
}
