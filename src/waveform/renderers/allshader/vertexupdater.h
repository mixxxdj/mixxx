#pragma once

namespace allshader {
class VertexUpdater;
}

class allshader::VertexUpdater {
  public:
    VertexUpdater(QVector2D* pData)
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
        *m_pWrite++ = QVector2D{x1, y1};
        *m_pWrite++ = QVector2D{x2, y2};
        *m_pWrite++ = QVector2D{x3, y3};
    }
    int index() const {
        return static_cast<int>(m_pWrite - m_pData);
    }

  private:
    QVector2D* const m_pData;
    QVector2D* m_pWrite;
};
