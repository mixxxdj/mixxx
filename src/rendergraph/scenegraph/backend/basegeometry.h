#pragma once

#include <QSGGeometry>

#include "rendergraph/attributeset.h"

namespace rendergraph {
class BaseGeometry;
}

class rendergraph::BaseGeometry : public QSGGeometry {
  protected:
    BaseGeometry(const AttributeSet& attributeSet, int vertexCount)
            : QSGGeometry(attributeSet, vertexCount),
              m_stride(attributeSet.stride) {
        QSGGeometry::setDrawingMode(QSGGeometry::DrawTriangleStrip);
    }
    const int m_stride;
};
