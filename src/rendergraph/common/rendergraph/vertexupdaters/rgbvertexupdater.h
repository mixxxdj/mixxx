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
            QVector3D rgb) {
        addRectangle(lt.x(), lt.y(), rb.x(), rb.y(), rgb.x(), rgb.y(), rgb.z());
    }
    void addRectangleVGradient(
            QVector2D lt,
            QVector2D rb,
            QVector3D rgbt,
            QVector3D rgbb) {
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
            QVector3D rgbl,
            QVector3D rgbr) {
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
    void addTriangle(QVector2D p1, QVector2D p2, QVector2D p3, QVector3D rgb) {
        addTriangle(p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y(), rgb.x(), rgb.y(), rgb.z());
    }
    void addTriangle(QVector2D p1,
            QVector2D p2,
            QVector2D p3,
            QVector3D rgb1,
            QVector3D rgb2,
            QVector3D rgb3) {
        addTriangle(p1.x(),
                p1.y(),
                p2.x(),
                p2.y(),
                p3.x(),
                p3.y(),
                rgb1.x(),
                rgb1.y(),
                rgb1.z(),
                rgb2.x(),
                rgb2.y(),
                rgb2.z(),
                rgb3.x(),
                rgb3.y(),
                rgb3.z());
    }
    int index() const {
        return static_cast<int>(m_pWrite - m_pData);
    }

  private:
    void addRectangle(float x1, float y1, float x2, float y2, float r, float g, float b) {
        addTriangle(x1, y1, x2, y1, x1, y2, r, g, b);
        addTriangle(x1, y2, x2, y2, x2, y1, r, g, b);
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
        addTriangle(x1, y1, x2, y1, x1, y2, r1, g1, b1, r1, g1, b1, r2, g2, b2);
        addTriangle(x1, y2, x2, y2, x2, y1, r2, g2, b2, r2, g2, b2, r1, g1, b1);
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
            float b2) {
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
        *m_pWrite++ = Geometry::RGBColoredPoint2D{{x1, y1}, {r, g, b}};
        *m_pWrite++ = Geometry::RGBColoredPoint2D{{x2, y2}, {r, g, b}};
        *m_pWrite++ = Geometry::RGBColoredPoint2D{{x3, y3}, {r, g, b}};
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
            float b3) {
        *m_pWrite++ = Geometry::RGBColoredPoint2D{{x1, y1}, {r1, g1, b1}};
        *m_pWrite++ = Geometry::RGBColoredPoint2D{{x2, y2}, {r2, g2, b2}};
        *m_pWrite++ = Geometry::RGBColoredPoint2D{{x3, y3}, {r3, g3, b3}};
    }

  private:
    Geometry::RGBColoredPoint2D* const m_pData;
    Geometry::RGBColoredPoint2D* m_pWrite;
};
