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

    Iterator iter = begin();

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

        GeometryNode* pNode;
        if (iter == end()) {
            qDebug() << "Appended new node for mark range";
            appendChildNode(std::make_unique<GeometryNode>());
            pNode = static_cast<GeometryNode*>(lastChildNode());
            pNode->setGeometry(std::make_unique<Geometry>(UniColorMaterial::attributes(), 0));
            pNode->setMaterial(std::make_unique<UniColorMaterial>());
            pNode->geometry().setDrawingMode(Geometry::DrawingMode::Triangles);
            pNode->geometry().allocate(6);

            iter = end();
        } else {
            pNode = static_cast<GeometryNode*>((*iter++).get());
        }

        VertexUpdater vertexUpdater{pNode->geometry().vertexDataAs<Geometry::Point2D>()};
        vertexUpdater.addRectangle(static_cast<float>(startPosition),
                0.f,
                static_cast<float>(endPosition) + 1.f,
                m_waveformRenderer->getBreadth());
        pNode->material().setUniform(0, matrix);
        pNode->material().setUniform(1, color);
    }
    while (iter != end()) {
        qDebug() << "Remove unused node for mark range";
        iter.incrementAfterRemove();
    }
}

} // namespace allshader
