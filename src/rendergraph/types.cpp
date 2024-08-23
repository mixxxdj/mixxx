#include "rendergraph/types.h"

#include <QMatrix4x4>
#include <QOpenGLFunctions>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>

using namespace rendergraph;

int rendergraph::sizeOf(PrimitiveType type) {
    switch (type) {
    case PrimitiveType::UInt:
        return sizeof(GLuint);
    case PrimitiveType::Float:
        return sizeof(GLfloat);
    }
    return 0;
}

int rendergraph::sizeOf(Type type) {
    switch (type) {
    case Type::UInt:
        return sizeof(GLuint);
    case Type::Float:
        return sizeof(GLfloat);
    case Type::Vector2D:
        return sizeof(QVector2D);
    case Type::Vector3D:
        return sizeof(QVector3D);
    case Type::Vector4D:
        return sizeof(QVector4D);
    case Type::Matrix4x4:
        return sizeof(QMatrix4x4);
    }
    return 0;
}

template<>
Type rendergraph::typeOf<GLuint>() {
    return Type::UInt;
}

template<>
Type rendergraph::typeOf<GLfloat>() {
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
PrimitiveType rendergraph::primitiveTypeOf<GLfloat>() {
    return PrimitiveType::Float;
}

template<>
PrimitiveType rendergraph::primitiveTypeOf<GLuint>() {
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
int rendergraph::tupleSizeOf<GLuint>() {
    return 1;
}
template<>
int rendergraph::tupleSizeOf<GLfloat>() {
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
