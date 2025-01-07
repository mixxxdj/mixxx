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
            QVector2D lt, QVector2D rb) {
        addRectangle(lt.x(), lt.y(), rb.x(), rb.y(), 0.f, 0.f, 1.f, 1.f);
    }
    void addRectangle(
            QVector2D lt, QVector2D rb, QVector2D tlr, QVector2D trb) {
        addRectangle(lt.x(), lt.y(), rb.x(), rb.y(), tlr.x(), tlr.y(), trb.x(), trb.y());
    }
    void addTriangle(QVector2D p1,
            QVector2D p2,
            QVector2D p3,
            QVector2D tp1,
            QVector2D tp2,
            QVector2D tp3) {
        addTriangle(p1.x(),
                p1.y(),
                p2.x(),
                p2.y(),
                p3.x(),
                p3.y(),
                tp1.x(),
                tp1.y(),
                tp2.x(),
                tp2.y(),
                tp3.x(),
                tp3.y());
    }
    int index() const {
        return static_cast<int>(m_pWrite - m_pData);
    }

  private:
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
        *m_pWrite++ = Geometry::TexturedPoint2D{{x1, y1}, {tx1, ty1}};
        *m_pWrite++ = Geometry::TexturedPoint2D{{x2, y2}, {tx2, ty2}};
        *m_pWrite++ = Geometry::TexturedPoint2D{{x3, y3}, {tx3, ty3}};
    }
    Geometry::TexturedPoint2D* const m_pData;
    Geometry::TexturedPoint2D* m_pWrite;
};
