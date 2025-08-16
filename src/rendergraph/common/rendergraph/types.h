#pragma once

namespace rendergraph {

enum class DrawingMode {
    Triangles,
    TriangleStrip
};

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
