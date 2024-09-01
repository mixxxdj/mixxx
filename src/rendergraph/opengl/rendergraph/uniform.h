#pragma once

#include <QString>

#include "rendergraph/types.h"

namespace rendergraph {
struct Uniform;
}

struct rendergraph::Uniform {
    const Type m_type;
    const QString m_name;

    Uniform(Type type)
            : m_type{type} {
    }

    Uniform(Type type, QString name)
            : m_type{type},
              m_name{std::move(name)} {
    }

    template<typename T>
    static Uniform create() {
        return Uniform(typeOf<T>());
    }
};
