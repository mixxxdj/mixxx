#pragma once

#include <QVector2D>
#include <QVector>

namespace qopengl {
class VertexData;
}

class qopengl::VertexData : public QVector<QVector2D> {
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
        push_back(a);
        push_back(b);
        push_back(c);
    }
};
