#pragma once

#include <QVector3D>
#include <QVector>

namespace qopengl {
class ColorData;
}

class qopengl::ColorData : public QVector<QVector3D> {
  public:
    void addForRectangle(float r, float g, float b) {
        push_back({r, g, b});
        push_back({r, g, b});
        push_back({r, g, b});
        push_back({r, g, b});
        push_back({r, g, b});
        push_back({r, g, b});
    }
};
