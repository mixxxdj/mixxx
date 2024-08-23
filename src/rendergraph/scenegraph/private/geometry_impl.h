#pragma once

#include <QSGGeometry>

#include "attributeset_impl.h"
#include "rendergraph/geometry.h"

class rendergraph::Geometry::Impl : public QSGGeometry {
  public:
    Impl(const rendergraph::AttributeSet& rgAttributeSet, int vertexCount)
            : QSGGeometry(rgAttributeSet.impl().sgAttributeSet(), vertexCount),
              m_stride(rgAttributeSet.impl().sgAttributeSet().stride) {
        QSGGeometry::setDrawingMode(QSGGeometry::DrawTriangleStrip);
    }

    QSGGeometry* sgGeometry() {
        return this;
    }

    void setAttributeValues(int attributePosition, const float* from, int numTuples) {
        const auto attributeArray = QSGGeometry::attributes();
        int offset = 0;
        for (int i = 0; i < attributePosition; i++) {
            offset += attributeArray[i].tupleSize;
        }
        const int tupleSize = attributeArray[attributePosition].tupleSize;
        const int skip = m_stride / sizeof(float) - tupleSize;

        float* to = static_cast<float*>(QSGGeometry::vertexData());
        to += offset;
        while (numTuples--) {
            int k = tupleSize;
            while (k--) {
                *to++ = *from++;
            }
            to += skip;
        }
    }

    void setDrawingMode(Geometry::DrawingMode mode) {
        QSGGeometry::setDrawingMode(toSgDrawingMode(mode));
    }

    Geometry::DrawingMode drawingMode() const {
        return fromSgDrawingMode(QSGGeometry::drawingMode());
    }

  private:
    const int m_stride;

    static QSGGeometry::DrawingMode toSgDrawingMode(Geometry::DrawingMode mode) {
        switch (mode) {
        case Geometry::DrawingMode::Triangles:
            return QSGGeometry::DrawTriangles;
        case Geometry::DrawingMode::TriangleStrip:
            return QSGGeometry::DrawTriangleStrip;
        }
    }
    static Geometry::DrawingMode fromSgDrawingMode(unsigned int mode) {
        switch (mode) {
        case QSGGeometry::DrawTriangles:
            return Geometry::DrawingMode::Triangles;
        case QSGGeometry::DrawTriangleStrip:
            return Geometry::DrawingMode::TriangleStrip;
        default:
            throw "not implemented";
        }
    }
};
