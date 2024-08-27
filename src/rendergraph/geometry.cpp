#include "rendergraph/geometry.h"

#include <QVector2D>

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

float* Geometry::vertexData() {
    return m_pImpl->vertexData();
}

template<>
QVector2D* Geometry::vertexDataAs<QVector2D>() {
    return m_pImpl->vertexDataAs<QVector2D>();
}

void Geometry::allocate(int vertexCount) {
    m_pImpl->allocate(vertexCount);
}

void Geometry::setDrawingMode(Geometry::DrawingMode mode) {
    m_pImpl->setDrawingMode(mode);
}

Geometry::DrawingMode Geometry::drawingMode() const {
    return m_pImpl->Impl::drawingMode();
}
