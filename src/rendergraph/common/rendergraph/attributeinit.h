#pragma once

#include "rendergraph/types.h"

namespace rendergraph {
struct AttributeInit;
}

struct rendergraph::AttributeInit {
    int m_tupleSize;
    PrimitiveType m_primitiveType;

    template<typename T>
    static AttributeInit create() {
        return AttributeInit{tupleSizeOf<T>(), primitiveTypeOf<T>()};
    }
};
