#pragma once

#include "rendergraph/geometry.h"

namespace rendergraph {
class TexturedVertexUpdater;
}

class rendergraph::TexturedVertexUpdater {
  public:
    TexturedVertexUpdater(Geometry::TexturedPoint2D* pData)
            : m_pData(pData),
              m_pWrite(pData) {
    }
    void addRectangle(
            QVector2D lt, QVector2D rb, QVector2D tlr, QVector2D trb) {
        addRectangle(lt.x(), lt.y(), rb.x(), rb.y(), tlr.x(), tlr.y(), trb.x(), trb.y());
    }
    void addRectangle(
            float x1, float y1, float x2, float y2, float tx1, float ty1, float tx2, float ty2) {
        addTriangle(x1, y1, x2, y1, x1, y2, tx1, ty1, tx2, ty1, tx1, ty2);
        addTriangle(x1, y2, x2, y2, x2, y1, tx1, ty2, tx2, ty2, tx2, ty1);
    }
    void addTriangle(float x1,
            float y1,
            float x2,
            float y2,
            float x3,
            float y3,
            float tx1,
            float ty1,
            float tx2,
            float ty2,
            float tx3,
            float ty3) {
        *m_pWrite++ = Geometry::TexturedPoint2D{x1, y1, tx1, ty1};
        *m_pWrite++ = Geometry::TexturedPoint2D{x2, y2, tx2, ty2};
        *m_pWrite++ = Geometry::TexturedPoint2D{x3, y3, tx3, ty3};
    }
    int index() const {
        return static_cast<int>(m_pWrite - m_pData);
    }

  private:
    Geometry::TexturedPoint2D* const m_pData;
    Geometry::TexturedPoint2D* m_pWrite;
};
