#include "waveform/renderers/waveformrendermarkrange.h"

#include <QPainter>
#include <QtDebug>

#include "util/painterscope.h"
#include "waveform/renderers/waveformwidgetrenderer.h"

WaveformRenderMarkRange::WaveformRenderMarkRange(WaveformWidgetRenderer* waveformWidgetRenderer) :
    WaveformRendererAbstract(waveformWidgetRenderer) {
}

void WaveformRenderMarkRange::setup(const QDomNode& node, const SkinContext& context) {
    m_markRanges.clear();
    m_markRanges.reserve(1);

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

void WaveformRenderMarkRange::draw(QPainter *painter, QPaintEvent * /*event*/) {
    PainterScope PainterScope(painter);

    painter->setWorldMatrixEnabled(false);

    if (isDirty()) {
        generateImages();
    }

    for (auto&& markRange: m_markRanges) {
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

        double startPosition = m_waveformRenderer->transformSamplePositionInRendererWorld(startSample);
        double endPosition = m_waveformRenderer->transformSamplePositionInRendererWorld(endSample);
        // The painter would extend the rectangle to the entire width while
        // span is < 1 AND span != 0.25 * [1;3], i.e. works with span = [.25; 0.75] px
        // TODO(xxx) Figure what's wrong with QPainter / the renderer
        const double span = std::max(endPosition - startPosition, 1.0);

        //range not in the current display
        if (startPosition > m_waveformRenderer->getLength() || endPosition < 0) {
            continue;
        }

        QImage* selectedImage = nullptr;

        selectedImage = markRange.enabled() ? &markRange.m_activeImage : &markRange.m_disabledImage;

        // draw the corresponding portion of the selected image
        // this shouldn't involve *any* scaling it should be fast even in software mode
        QRectF rect;
        if (m_waveformRenderer->getOrientation() == Qt::Horizontal) {
            rect.setRect(startPosition, 0, span, m_waveformRenderer->getHeight());
        } else {
            rect.setRect(0, startPosition, m_waveformRenderer->getWidth(), span);
        }
        painter->drawImage(rect, *selectedImage, rect);
    }
}

void WaveformRenderMarkRange::generateImages() {
    for (auto&& markRange: m_markRanges) {
        markRange.generateImage(m_waveformRenderer->getWidth(), m_waveformRenderer->getHeight());
    }
    setDirty(false);
}
