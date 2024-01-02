#pragma once

#include <QOpenGLBuffer>
#include <cassert>

namespace allshader {
class VertexBuffer;
class Vector2DVertexBuffer;
class Vector2DRGBVertexBuffer;
class Vector2DRGBAVertexBuffer;
}

class allshader::VertexBuffer : public QOpenGLBuffer {
  public:
    VertexBuffer(int tupleSize)
            : QOpenGLBuffer(QOpenGLBuffer::VertexBuffer), m_tupleSize(tupleSize) {
    }

    void reserve(int count) {
        int nBytes = count * stride() * sizeof(float);
        if (nBytes > m_allocatedBytes) {
            allocate(nBytes);
            m_allocatedBytes = nBytes;
        }
    }

    void mapForWrite() {
        void* ptr = QOpenGLBuffer::map(QOpenGLBuffer::WriteOnly);
        assert(ptr);
        m_pMapped = static_cast<float*>(ptr);
        m_valueIndex = 0;
    }

    void setIndex(int index) { // in amount of tuples
        m_valueIndex = index * m_tupleSize;
    }

    int tupleSize() const { // in amount of floats
        return m_tupleSize;
    }

    int size() const { // in tuples
        return m_valueIndex / m_tupleSize;
    }

    int stride() const { // in bytes
        return m_tupleSize * sizeof(float);
    }

  protected:
    const int m_tupleSize;
    int m_allocatedBytes{};
    float* m_pMapped{};
    int m_valueIndex{}; // index into m_pMapped, in amount of floats
};

class allshader::Vector2DVertexBuffer : public allshader::VertexBuffer {
  public:
    Vector2DVertexBuffer()
            : VertexBuffer{2} {
    }

    constexpr int offset() const {
        return 0;
    }

    void add(float x, float y) {
        m_pMapped[m_valueIndex++] = x;
        m_pMapped[m_valueIndex++] = y;
    }

    void addRectangle(float x1, float y1, float x2, float y2) {
        add(x1, y1);
        add(x2, y1);
        add(x1, y2);

        add(x1, y2);
        add(x2, y2);
        add(x2, y1);
    }
};

class allshader::Vector2DRGBVertexBuffer : public VertexBuffer {
  public:
    Vector2DRGBVertexBuffer()
            : VertexBuffer{positionTupleSize() + colorTupleSize()} {
    }

    constexpr int positionOffset() const {
        return 0;
    }

    constexpr int positionTupleSize() const {
        return 2;
    }

    constexpr int colorOffset() const {
        return positionTupleSize() * sizeof(float);
    }

    constexpr int colorTupleSize() const {
        return 3;
    }

    void add(float x, float y, float r, float g, float b) {
        m_pMapped[m_valueIndex++] = x;
        m_pMapped[m_valueIndex++] = y;

        m_pMapped[m_valueIndex++] = r;
        m_pMapped[m_valueIndex++] = g;
        m_pMapped[m_valueIndex++] = b;
    }

    void addRectangle(float x1, float y1, float x2, float y2, float r, float g, float b) {
        add(x1, y1, r, g, b);
        add(x2, y1, r, g, b);
        add(x1, y2, r, g, b);

        add(x1, y2, r, g, b);
        add(x2, y2, r, g, b);
        add(x2, y1, r, g, b);
    }
};

class allshader::Vector2DRGBAVertexBuffer : public VertexBuffer {
  public:
    Vector2DRGBAVertexBuffer()
            : VertexBuffer{positionTupleSize() + colorTupleSize()} {
    }

    constexpr int positionOffset() const {
        return 0;
    }

    constexpr int positionTupleSize() const {
        return 2;
    }

    constexpr int colorOffset() const {
        return positionTupleSize() * sizeof(float);
    }

    constexpr int colorTupleSize() const {
        return 4;
    }

    void add(float x, float y, float r, float g, float b, float a) {
        m_pMapped[m_valueIndex++] = x;
        m_pMapped[m_valueIndex++] = y;

        m_pMapped[m_valueIndex++] = r;
        m_pMapped[m_valueIndex++] = g;
        m_pMapped[m_valueIndex++] = b;
        m_pMapped[m_valueIndex++] = a;
    }

    void addRectangle(float x1, float y1, float x2, float y2, float r, float g, float b, float a) {
        add(x1, y1, r, g, b, a);
        add(x2, y1, r, g, b, a);
        add(x1, y2, r, g, b, a);

        add(x1, y2, r, g, b, a);
        add(x2, y2, r, g, b, a);
        add(x2, y1, r, g, b, a);
    }

    void addRectangleGradient(float x1,
            float y1,
            float x2,
            float y2,
            float rA,
            float gA,
            float bA,
            float aA,
            float rB,
            float gB,
            float bB,
            float aB) {
        add(x1, y1, rA, gA, bA, aA);
        add(x2, y1, rA, gA, bA, aA);
        add(x1, y2, rB, gB, bB, aB);
        add(x1, y2, rB, gB, bB, aB);
        add(x2, y2, rB, gB, bB, aB);
        add(x2, y1, rA, gA, bA, aA);
    }
};
