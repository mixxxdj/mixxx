#pragma once

#include <QString>

#include "rendergraph/types.h"

namespace rendergraph {
struct Attribute;
}

struct rendergraph::Attribute {
    const int m_tupleSize;
    const PrimitiveType m_primitiveType;
    const QString m_name;

    Attribute(int tupleSize, PrimitiveType primitiveType, QString name = {})
            : m_tupleSize{tupleSize},
              m_primitiveType{primitiveType},
              m_name{std::move(name)} {
    }

    template<typename T>
    static Attribute create() {
        return Attribute(tupleSizeOf<T>(), primitiveTypeOf<T>());
    }
};
