#pragma once

#include <QSGGeometry>

#include "rendergraph/attributeset.h"

namespace rendergraph::backend {
class Geometry;
}

class rendergraph::backend::Geometry : public QSGGeometry {
  protected:
    Geometry(const rendergraph::AttributeSet& attributeSet, int vertexCount)
            : QSGGeometry(attributeSet, vertexCount),
              m_stride(attributeSet.stride) {
        QSGGeometry::setDrawingMode(QSGGeometry::DrawTriangleStrip);
    }
    const int m_stride;
};
