#pragma once

#include <QSGGeometry>
#include <initializer_list>
#include <vector>

#include "rendergraph/attributeinit.h"

namespace rendergraph {
class BaseAttributeSet;
class BaseAttributeSetHelper;
} // namespace rendergraph

class rendergraph::BaseAttributeSetHelper {
  protected:
    BaseAttributeSetHelper(std::initializer_list<AttributeInit> list);
    std::vector<QSGGeometry::Attribute> m_sgAttributes;
};

class rendergraph::BaseAttributeSet
        : protected rendergraph::BaseAttributeSetHelper,
          public QSGGeometry::AttributeSet {
  protected:
    BaseAttributeSet(std::initializer_list<AttributeInit> list)
            : BaseAttributeSetHelper(list),
              QSGGeometry::AttributeSet{static_cast<int>(m_sgAttributes.size()),
                      calculateSizeOfVertex(list),
                      m_sgAttributes.data()} {
    }
    static int calculateSizeOfVertex(std::initializer_list<AttributeInit> list) {
        int numBytes = 0;
        for (auto item : list) {
            numBytes += item.m_tupleSize * sizeOf(item.m_primitiveType);
        }
        return numBytes;
    }
};
