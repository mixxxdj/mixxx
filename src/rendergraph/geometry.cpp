#include "rendergraph/geometry.h"

#include "geometry_impl.h"

using namespace rendergraph;

Geometry::Geometry(Impl* pImpl)
        : m_pImpl(pImpl) {
}

Geometry::Geometry(const AttributeSet& attributeSet, int vertexCount)
        : Geometry(new Geometry::Impl(attributeSet, vertexCount)){};

Geometry::~Geometry() = default;

Geometry::Impl& Geometry::impl() const {
    return *m_pImpl;
}

void Geometry::setAttributeValues(int attributePosition, const float* data, int numTuples) {
    m_pImpl->setAttributeValues(attributePosition, data, numTuples);
}

void Geometry::setDrawingMode(Geometry::DrawingMode mode) {
    m_pImpl->setDrawingMode(mode);
}

Geometry::DrawingMode Geometry::drawingMode() const {
    return m_pImpl->Impl::drawingMode();
}
