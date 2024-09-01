#pragma once

#include <QMatrix4x4>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>

namespace rendergraph {

enum class PrimitiveType {
    UInt,
    Float,
};

enum class Type {
    UInt,
    Float,
    Vector2D,
    Vector3D,
    Vector4D,
    Matrix4x4
};

int sizeOf(Type type);
int sizeOf(PrimitiveType primitiveType);

template<typename T>
Type typeOf();

template<typename T>
PrimitiveType primitiveTypeOf();

template<typename T>
int tupleSizeOf();

} // namespace rendergraph
