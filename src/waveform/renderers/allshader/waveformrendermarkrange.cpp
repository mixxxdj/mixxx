#include "waveform/renderers/allshader/waveformrendermarkrange.h"

#include "skin/legacy/skincontext.h"
#include "waveform/renderers/allshader/matrixforwidgetgeometry.h"
#include "waveform/renderers/waveformwidgetrenderer.h"

allshader::WaveformRenderMarkRange::WaveformRenderMarkRange(WaveformWidgetRenderer* waveformWidget)
        : WaveformRenderer(waveformWidget) {
}

void allshader::WaveformRenderMarkRange::initializeGL() {
    WaveformRenderer::initializeGL();
    m_shader.init();
}

void allshader::WaveformRenderMarkRange::fillRect(
        const QRectF& rect, QColor color) {
    const float posx1 = static_cast<float>(rect.x());
    const float posx2 = static_cast<float>(rect.x() + rect.width());
    const float posy1 = static_cast<float>(rect.y());
    const float posy2 = static_cast<float>(rect.y() + rect.height());

    const float posarray[] = {posx1, posy1, posx2, posy1, posx1, posy2, posx2, posy2};

    const int colorLocation = m_shader.colorLocation();
    const int positionLocation = m_shader.positionLocation();

    m_shader.setUniformValue(colorLocation, color);

    m_shader.setAttributeArray(
            positionLocation, GL_FLOAT, posarray, 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void allshader::WaveformRenderMarkRange::setup(const QDomNode& node, const SkinContext& context) {
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

void allshader::WaveformRenderMarkRange::paintGL() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const QMatrix4x4 matrix = matrixForWidgetGeometry(m_waveformRenderer, false);

    const int positionLocation = m_shader.positionLocation();
    const int matrixLocation = m_shader.matrixLocation();

    m_shader.bind();
    m_shader.enableAttributeArray(positionLocation);

    m_shader.setUniformValue(matrixLocation, matrix);

    for (auto&& markRange : m_markRanges) {
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

        const double span = std::max(endPosition - startPosition, 1.0);

        // range not in the current display
        if (startPosition > m_waveformRenderer->getLength() || endPosition < 0) {
            continue;
        }

        QColor color = markRange.enabled() ? markRange.m_activeColor : markRange.m_disabledColor;
        color.setAlphaF(0.3f);

        fillRect(QRectF(startPosition, 0, span, m_waveformRenderer->getBreadth()), color);
    }
    m_shader.disableAttributeArray(positionLocation);
    m_shader.release();
}
