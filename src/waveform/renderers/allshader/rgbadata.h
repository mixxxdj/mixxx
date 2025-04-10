#pragma once

#include <QVector4D>
#include <QVector>

namespace allshader {
class RGBAData;
} // namespace allshader

class allshader::RGBAData {
    QVector<QVector4D> mData;

  public:
    void addForRectangle(float r, float g, float b, float a) {
        mData.push_back({r, g, b, a});
        mData.push_back({r, g, b, a});
        mData.push_back({r, g, b, a});
        mData.push_back({r, g, b, a});
        mData.push_back({r, g, b, a});
        mData.push_back({r, g, b, a});
    }
    void addForRectangleGradient(float rA,
            float gA,
            float bA,
            float aA,
            float rB,
            float gB,
            float bB,
            float aB) {
        mData.push_back({rA, gA, bA, aA});
        mData.push_back({rA, gA, bA, aA});
        mData.push_back({rB, gB, bB, aB});
        mData.push_back({rB, gB, bB, aB});
        mData.push_back({rB, gB, bB, aB});
        mData.push_back({rA, gA, bA, aA});
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
    const QVector4D* constData() const {
        return mData.constData();
    }
};
