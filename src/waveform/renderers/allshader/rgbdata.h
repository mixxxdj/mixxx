#pragma once

#include <QVector3D>
#include <QVector>

namespace allshader {
class RGBData;
} // namespace allshader

class allshader::RGBData {
    QVector<QVector3D> mData;

  public:
    void addForRectangle(float r, float g, float b) {
        mData.push_back({r, g, b});
        mData.push_back({r, g, b});
        mData.push_back({r, g, b});
        mData.push_back({r, g, b});
        mData.push_back({r, g, b});
        mData.push_back({r, g, b});
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
    const QVector3D* constData() const {
        return mData.constData();
    }
};
