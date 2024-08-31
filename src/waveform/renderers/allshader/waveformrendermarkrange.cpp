#include "waveform/renderers/allshader/waveformrendermarkrange.h"

#include "rendergraph/geometry.h"
#include "rendergraph/geometrynode.h"
#include "rendergraph/material/unicolormaterial.h"
#include "skin/legacy/skincontext.h"
#include "vertexupdater.h"
#include "waveform/renderers/allshader/matrixforwidgetgeometry.h"
#include "waveform/renderers/waveformwidgetrenderer.h"

using namespace rendergraph;

namespace allshader {

WaveformRenderMarkRange::WaveformRenderMarkRange(WaveformWidgetRenderer* waveformWidget)
        : ::WaveformRendererAbstract(waveformWidget) {
}

void WaveformRenderMarkRange::setup(const QDomNode& node, const SkinContext& context) {
    m_markRanges.clear();

    QDomNode child = node.firstChild();
    while (!child.isNull()) {
        if (child.nodeName() == "MarkRange") {
            m_markRanges.push_back(
                    WaveformMarkRange(
                            m_waveformRenderer->getGroup(),
                            child,
                            context,
                            *m_waveformRenderer->getWaveformSignalColors()));
        }
        child = child.nextSibling();
    }
}

void WaveformRenderMarkRange::draw(QPainter* painter, QPaintEvent* event) {
    Q_UNUSED(painter);
    Q_UNUSED(event);
    DEBUG_ASSERT(false);
}

void WaveformRenderMarkRange::updateNode() {
    const QMatrix4x4 matrix = matrixForWidgetGeometry(m_waveformRenderer, false);

    GeometryNode* pChild = static_cast<GeometryNode*>(firstChild());

    for (const auto& markRange : m_markRanges) {
        // If the mark range is not active we should not draw it.
        if (!markRange.active()) {
            continue;
        }

        // If the mark range is not visible we should not draw it.
        if (!markRange.visible()) {
            continue;
        }

        // Active mark ranges by definition have starts/ends that are not
        // disabled so no need to check.
        double startSample = markRange.start();
        double endSample = markRange.end();

        double startPosition =
                m_waveformRenderer->transformSamplePositionInRendererWorld(
                        startSample);
        double endPosition = m_waveformRenderer->transformSamplePositionInRendererWorld(endSample);

        startPosition = std::floor(startPosition);
        endPosition = std::floor(endPosition);

        // range not in the current display
        if (startPosition > m_waveformRenderer->getLength() || endPosition < 0) {
            continue;
        }

        QColor color = markRange.enabled() ? markRange.m_activeColor : markRange.m_disabledColor;
        color.setAlphaF(0.3f);

        if (!pChild) {
            qDebug() << "Appended new node for mark range";
            appendChildNode(std::make_unique<GeometryNode>());
            pChild = static_cast<GeometryNode*>(lastChild());
            pChild->setGeometry(std::make_unique<Geometry>(UniColorMaterial::attributes(), 0));
            pChild->setMaterial(std::make_unique<UniColorMaterial>());
            pChild->geometry().setDrawingMode(Geometry::DrawingMode::Triangles);
            pChild->geometry().allocate(6);
        }

        VertexUpdater vertexUpdater{pChild->geometry().vertexDataAs<Geometry::Point2D>()};
        vertexUpdater.addRectangle(static_cast<float>(startPosition),
                0.f,
                static_cast<float>(endPosition) + 1.f,
                m_waveformRenderer->getBreadth());
        pChild->material().setUniform(0, matrix);
        pChild->material().setUniform(1, color);

        pChild = static_cast<GeometryNode*>(pChild->nextSibling());
    }
    while (pChild) {
        qDebug() << "Remove unused node for mark range";
        auto pNext = static_cast<GeometryNode*>(pChild->nextSibling());
        removeChildNode(pChild);
        pChild = pNext;
    }
}

} // namespace allshader
