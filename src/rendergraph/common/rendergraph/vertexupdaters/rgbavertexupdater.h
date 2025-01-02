#pragma once

#include "rendergraph/geometry.h"

namespace rendergraph {
class RGBAVertexUpdater;
}

class rendergraph::RGBAVertexUpdater {
  public:
    RGBAVertexUpdater(Geometry::RGBAColoredPoint2D* pData)
            : m_pData(pData),
              m_pWrite(pData) {
    }

    void addRectangle(
            QVector2D lt,
            QVector2D rb,
            QVector4D rgba) {
        addRectangle(lt.x(), lt.y(), rb.x(), rb.y(), rgba.x(), rgba.y(), rgba.z(), rgba.w());
    }
    void addRectangleVGradient(
            QVector2D lt,
            QVector2D rb,
            QVector4D rgbat,
            QVector4D rgbab) {
        addRectangleVGradient(lt.x(),
                lt.y(),
                rb.x(),
                rb.y(),
                rgbat.x(),
                rgbat.y(),
                rgbat.z(),
                rgbat.w(),
                rgbab.x(),
                rgbab.y(),
                rgbab.z(),
                rgbab.w());
    }
    void addRectangleHGradient(
            QVector2D lt,
            QVector2D rb,
            QVector4D rgbal,
            QVector4D rgbar) {
        addRectangleHGradient(lt.x(),
                lt.y(),
                rb.x(),
                rb.y(),
                rgbal.x(),
                rgbal.y(),
                rgbal.z(),
                rgbal.w(),
                rgbar.x(),
                rgbar.y(),
                rgbar.z(),
                rgbar.w());
    }
    int index() const {
        return static_cast<int>(m_pWrite - m_pData);
    }
    void addTriangle(QVector2D p1, QVector2D p2, QVector2D p3, QVector4D rgba) {
        addTriangle(p1.x(),
                p1.y(),
                p2.x(),
                p2.y(),
                p3.x(),
                p3.y(),
                rgba.x(),
                rgba.y(),
                rgba.z(),
                rgba.w());
    }
    void addTriangle(QVector2D p1,
            QVector2D p2,
            QVector2D p3,
            QVector4D rgba1,
            QVector4D rgba2,
            QVector4D rgba3) {
        addTriangle(p1.x(),
                p1.y(),
                p2.x(),
                p2.y(),
                p3.x(),
                p3.y(),
                rgba1.x(),
                rgba1.y(),
                rgba1.z(),
                rgba1.w(),
                rgba2.x(),
                rgba2.y(),
                rgba2.z(),
                rgba2.w(),
                rgba3.x(),
                rgba3.y(),
                rgba3.z(),
                rgba3.w());
    }

  private:
    void addRectangle(float x1, float y1, float x2, float y2, float r, float g, float b, float a) {
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
            float a1,
            float r2,
            float g2,
            float b2,
            float a2) {
        addTriangle(x1, y1, x2, y1, x1, y2, r1, g1, b1, a1, r1, g1, b1, a1, r2, g2, b2, a2);
        addTriangle(x1, y2, x2, y2, x2, y1, r2, g2, b2, a2, r2, g2, b2, a2, r1, g1, b1, a1);
    }
    void addRectangleHGradient(
            float x1,
            float y1,
            float x2,
            float y2,
            float r1,
            float g1,
            float b1,
            float a1,
            float r2,
            float g2,
            float b2,
            float a2) {
        addTriangle(x1, y1, x2, y1, x1, y2, r1, g1, b1, a1, r2, g2, b2, a2, r1, g1, b1, a1);
        addTriangle(x1, y2, x2, y2, x2, y1, r1, g1, b1, a1, r2, g2, b2, a2, r2, g2, b2, a2);
    }
    void addTriangle(float x1,
            float y1,
            float x2,
            float y2,
            float x3,
            float y3,
            float r,
            float g,
            float b,
            float a) {
        *m_pWrite++ = Geometry::RGBAColoredPoint2D{{x1, y1}, {r, g, b, a}};
        *m_pWrite++ = Geometry::RGBAColoredPoint2D{{x2, y2}, {r, g, b, a}};
        *m_pWrite++ = Geometry::RGBAColoredPoint2D{{x3, y3}, {r, g, b, a}};
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
            float a1,
            float r2,
            float g2,
            float b2,
            float a2,
            float r3,
            float g3,
            float b3,
            float a3) {
        *m_pWrite++ = Geometry::RGBAColoredPoint2D{{x1, y1}, {r1, g1, b1, a1}};
        *m_pWrite++ = Geometry::RGBAColoredPoint2D{{x2, y2}, {r2, g2, b2, a2}};
        *m_pWrite++ = Geometry::RGBAColoredPoint2D{{x3, y3}, {r3, g3, b3, a3}};
    }
    Geometry::RGBAColoredPoint2D* const m_pData;
    Geometry::RGBAColoredPoint2D* m_pWrite;
};
