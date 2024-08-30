#pragma once

namespace rendergraph {
class VertexUpdater;
}

class rendergraph::VertexUpdater {
  public:
    VertexUpdater(Geometry::Point2D* pData)
            : m_pData(pData),
              m_pWrite(pData) {
    }

    void addRectangle(
            float x1,
            float y1,
            float x2,
            float y2) {
        addTriangle(x1, y1, x2, y1, x1, y2);
        addTriangle(x1, y2, x2, y2, x2, y1);
    }
    void addTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
        *m_pWrite++ = Geometry::Point2D{x1, y1};
        *m_pWrite++ = Geometry::Point2D{x2, y2};
        *m_pWrite++ = Geometry::Point2D{x3, y3};
    }
    int index() const {
        return static_cast<int>(m_pWrite - m_pData);
    }

  private:
    Geometry::Point2D* const m_pData;
    Geometry::Point2D* m_pWrite;
};
