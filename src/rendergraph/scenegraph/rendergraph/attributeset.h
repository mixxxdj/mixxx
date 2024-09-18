#pragma once

#include <QSGGeometry>
#include <initializer_list>
#include <vector>

#include "rendergraph/attribute.h"

namespace rendergraph {
class AttributeSetBase;
class AttributeSet;
}

class rendergraph::AttributeSetBase {
  protected:
    int m_stride{};

    AttributeSetBase(std::initializer_list<Attribute> list, const std::vector<QString>& names);

  private:
    void add(const Attribute& attribute);
    static int toQSGGeometryType(const rendergraph::PrimitiveType& t);

  protected:
    std::vector<Attribute> m_attributes;
    QSGGeometry::AttributeSet m_sgAttributeSet{};
    std::vector<QSGGeometry::Attribute> m_sgAttributes;
};

class rendergraph::AttributeSet : private rendergraph::AttributeSetBase,
                                  public QSGGeometry::AttributeSet {
  public:
    AttributeSet(std::initializer_list<Attribute> list, const std::vector<QString>& names);
    ~AttributeSet();

    const std::vector<Attribute>& attributes() const {
        return m_attributes;
    }
    const QSGGeometry::AttributeSet& sgAttributeSet() const {
        return *this;
    }
};

namespace rendergraph {
template<typename... T>
AttributeSet makeAttributeSet(const std::vector<QString>& names) {
    return AttributeSet({(Attribute::create<T>())...}, names);
}
} // namespace rendergraph
