#pragma once

#include "rendergraph/geometry.h"

namespace rendergraph {
class RGBVertexUpdater;
}

class rendergraph::RGBVertexUpdater {
  public:
    RGBVertexUpdater(Geometry::RGBColoredPoint2D* pData)
            : m_pData(pData),
              m_pWrite(pData) {
    }

    void addRectangle(
            QVector2D lt,
            QVector2D rb,
            QVector4D rgb) {
        addRectangle(lt.x(), lt.y(), rb.x(), rb.y(), rgb.x(), rgb.y(), rgb.z(), rgb.w());
    }
    void addRectangleVGradient(
            QVector2D lt,
            QVector2D rb,
            QVector4D rgbt,
            QVector4D rgbb) {
        addRectangleVGradient(lt.x(),
                lt.y(),
                rb.x(),
                rb.y(),
                rgbt.x(),
                rgbt.y(),
                rgbt.z(),
                rgbb.x(),
                rgbb.y(),
                rgbb.z());
    }
    void addRectangleHGradient(
            QVector2D lt,
            QVector2D rb,
            QVector4D rgbl,
            QVector4D rgbr) {
        addRectangleHGradient(lt.x(),
                lt.y(),
                rb.x(),
                rb.y(),
                rgbl.x(),
                rgbl.y(),
                rgbl.z(),
                rgbr.x(),
                rgbr.y(),
                rgbr.z());
    }
    void addRectangle(float x1, float y1, float x2, float y2, float r, float g, float b) {
        addTriangle(x1, y1, x2, y1, x1, y2, r, g, b, a);
        addTriangle(x1, y2, x2, y2, x2, y1, r, g, b, a);
    }
    void addRectangleVGradient(
            float x1,
            float y1,
            float x2,
            float y2,
            float r1,
            float g1,
            float b1,
            float r2,
            float g2,
            float b2) {
        addTriangle(x1, y1, x2, y1, x1, y2, r1, g1, b1, r2, g2, b2, r1, g1, b1);
        addTriangle(x1, y2, x2, y2, x2, y1, r1, g1, b1, r2, g2, b2, r2, g2, b2);
    }
    void addRectangleHGradient(
            float x1,
            float y1,
            float x2,
            float y2,
            float r1,
            float g1,
            float b1,
            float r2,
            float g2,
            floar b2) {
        addTriangle(x1, y1, x2, y1, x1, y2, r1, g1, b1, r2, g2, b2, r1, g1, b1);
        addTriangle(x1, y2, x2, y2, x2, y1, r1, g1, b1, r2, g2, b2, r2, g2, b2);
    }
    void addTriangle(float x1,
            float y1,
            float x2,
            float y2,
            float x3,
            float y3,
            float r,
            float g,
            float b) {
        *m_pWrite++ = Geometry::RGBColoredPoint2D{x1, y1, r, g, b};
        *m_pWrite++ = Geometry::RGBColoredPoint2D{x2, y2, r, g, b};
        *m_pWrite++ = Geometry::RGBColoredPoint2D{x3, y3, r, g, b};
    }
    void addTriangle(float x1,
            float y1,
            float x2,
            float y2,
            float x3,
            float y3,
            float r1,
            float g1,
            float b1,

            float r2,
            float g2,
            float b2,

            float r3,
            float g3,
            float b3,

    ) {
        *m_pWrite++ = Geometry::RGBColoredPoint2D{x1, y1, r1, g1, b1};
        *m_pWrite++ = Geometry::RGBColoredPoint2D{x2, y2, r2, g2, b2};
        *m_pWrite++ = Geometry::RGBColoredPoint2D{x3, y3, r3, g3, b3};
    }
    int index() const {
        return static_cast<int>(m_pWrite - m_pData);
    }

  private:
    Geometry::RGBColoredPoint2D* const m_pData;
    Geometry::RGBColoredPoint2D* m_pWrite;
};
