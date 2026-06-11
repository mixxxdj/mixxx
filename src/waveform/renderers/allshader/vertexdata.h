#pragma once

#include <QVector2D>
#include <QVector>

namespace allshader {
class VertexData;
} // namespace allshader

class allshader::VertexData {
    QVector<QVector2D> mData;

  public:
    void addRectangle(
            float x1,
            float y1,
            float x2,
            float y2) {
        addTriangle({x1, y1}, {x2, y1}, {x1, y2});
        addTriangle({x1, y2}, {x2, y2}, {x2, y1});
    }
    void addTriangle(const QVector2D& a, const QVector2D& b, const QVector2D& c) {
        mData.push_back(a);
        mData.push_back(b);
        mData.push_back(c);
    }
    void clear() {
        mData.clear();
    }
    void reserve(int size) {
        mData.reserve(size);
    }
    int size() const {
        return mData.size();
    }
    const QVector2D* constData() const {
        return mData.constData();
    }
};
