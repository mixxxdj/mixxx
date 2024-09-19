#include "rendergraph/types.h"

#include <QMatrix4x4>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <cstdint>

using namespace rendergraph;

int rendergraph::sizeOf(PrimitiveType type) {
    switch (type) {
    case PrimitiveType::UInt:
        return sizeof(uint32_t);
    case PrimitiveType::Float:
        return sizeof(float);
    }
    return 0;
}

int rendergraph::sizeOf(Type type) {
    switch (type) {
    case Type::UInt:
        return sizeof(uint32_t);
    case Type::Float:
        return sizeof(float);
    case Type::Vector2D:
        return sizeof(float) * 2;
    case Type::Vector3D:
        return sizeof(float) * 3;
    case Type::Vector4D:
        return sizeof(float) * 4;
    case Type::Matrix4x4:
        return sizeof(float) * 4 * 4;
    }
    return 0;
}

template<>
Type rendergraph::typeOf<uint32_t>() {
    return Type::UInt;
}

template<>
Type rendergraph::typeOf<float>() {
    return Type::Float;
}

template<>
Type rendergraph::typeOf<QVector2D>() {
    return Type::Vector2D;
}

template<>
Type rendergraph::typeOf<QVector3D>() {
    return Type::Vector3D;
}

template<>
Type rendergraph::typeOf<QVector4D>() {
    return Type::Vector4D;
}

template<>
Type rendergraph::typeOf<QMatrix4x4>() {
    return Type::Matrix4x4;
}

template<>
PrimitiveType rendergraph::primitiveTypeOf<float>() {
    return PrimitiveType::Float;
}

template<>
PrimitiveType rendergraph::primitiveTypeOf<uint32_t>() {
    return PrimitiveType::UInt;
}

template<>
PrimitiveType rendergraph::primitiveTypeOf<QVector2D>() {
    return PrimitiveType::Float;
}
template<>
PrimitiveType rendergraph::primitiveTypeOf<QVector3D>() {
    return PrimitiveType::Float;
}
template<>
PrimitiveType rendergraph::primitiveTypeOf<QVector4D>() {
    return PrimitiveType::Float;
}

template<>
int rendergraph::tupleSizeOf<uint32_t>() {
    return 1;
}
template<>
int rendergraph::tupleSizeOf<float>() {
    return 1;
}
template<>
int rendergraph::tupleSizeOf<QVector2D>() {
    return 2;
}
template<>
int rendergraph::tupleSizeOf<QVector3D>() {
    return 3;
}
template<>
int rendergraph::tupleSizeOf<QVector4D>() {
    return 4;
}
