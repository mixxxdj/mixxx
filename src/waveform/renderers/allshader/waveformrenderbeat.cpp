#include "waveform/renderers/allshader/waveformrenderbeat.h"

#include <QDomNode>

#include "skin/legacy/skincontext.h"
#include "track/track.h"
#include "waveform/renderers/allshader/matrixforwidgetgeometry.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "widget/wskincolor.h"

namespace allshader {

WaveformRenderBeat::WaveformRenderBeat(WaveformWidgetRenderer* waveformWidget,
        ::WaveformRendererAbstract::PositionSource type,
        WaveformRendererSignalBase::Options options)
        : WaveformRenderer(waveformWidget),
          m_isSlipRenderer(type == ::WaveformRendererAbstract::Slip),
          m_displayDownbeat(options &
                  allshader::WaveformRendererSignalBase::Option::BeatGridBar),
          m_displayMarkerbeat(options &
                  allshader::WaveformRendererSignalBase::Option::
                          BeatGridMarker) {
}

void WaveformRenderBeat::initializeGL() {
    WaveformRenderer::initializeGL();
    m_beatShader.init();
    if (m_displayDownbeat) {
        m_downbeatShader.init();
    }
    if (m_displayMarkerbeat) {
        m_markerbeatShader.init();
    }
}

void WaveformRenderBeat::setup(const QDomNode& node, const SkinContext& context) {
    m_beatColor = QColor(context.selectString(node, "BeatColor"));
    m_beatColor = WSkinColor::getCorrectColor(m_beatColor).toRgb();

    if (m_displayDownbeat) {
        m_downbeatColor.setNamedColor(context.selectString(node, "DownbeatColor"));
        m_downbeatColor = WSkinColor::getCorrectColor(m_downbeatColor).toRgb();
    }

    if (m_displayMarkerbeat) {
        m_markerbeatColor.setNamedColor(context.selectString(node, "MarkerbeatColor"));
        m_markerbeatColor = WSkinColor::getCorrectColor(m_markerbeatColor).toRgb();
    }
}

void WaveformRenderBeat::paintGL() {
    TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();

    if (!trackInfo || (m_isSlipRenderer && !m_waveformRenderer->isSlipActive())) {
        return;
    }

    auto positionType = m_isSlipRenderer ? ::WaveformRendererAbstract::Slip
                                         : ::WaveformRendererAbstract::Play;

    mixxx::BeatsPointer trackBeats = trackInfo->getBeats();
    if (!trackBeats) {
        return;
    }

    int alpha = m_waveformRenderer->getBeatGridAlpha();
    if (alpha == 0) {
        return;
    }

    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    QColor beatColor = m_beatColor,
           downbeatColor = m_downbeatColor,
           markerbeatColor = m_markerbeatColor;
    beatColor.setAlphaF(alpha / 100.0f * beatColor.alphaF());
    downbeatColor.setAlphaF(alpha / 100.0f * downbeatColor.alphaF());
    markerbeatColor.setAlphaF(alpha / 100.0f * markerbeatColor.alphaF());

    const double trackSamples = m_waveformRenderer->getTrackSamples();
    if (trackSamples <= 0) {
        return;
    }

    const double firstDisplayedPosition =
            m_waveformRenderer->getFirstDisplayedPosition(positionType);
    const double lastDisplayedPosition =
            m_waveformRenderer->getLastDisplayedPosition(positionType);

    const auto startPosition = mixxx::audio::FramePos::fromEngineSamplePos(
            firstDisplayedPosition * trackSamples);
    const auto endPosition = mixxx::audio::FramePos::fromEngineSamplePos(
            lastDisplayedPosition * trackSamples);

    if (!startPosition.isValid() || !endPosition.isValid()) {
        return;
    }

    const float rendererBreadth = m_waveformRenderer->getBreadth();

    const int numVerticesPerLine = 6; // 2 triangles

    // Count the number of beats in the range to reserve space in the m_vertices vector.
    // Note that we could also use
    //   int numBearsInRange = trackBeats->numBeatsInRange(startPosition, endPosition);
    // for this, but there have been reports of that method failing with a DEBUG_ASSERT.
    int numBeatsInRange = 0, numDownbeatsInRange = 0, numMarkerbeatsInRange = 0;
    for (auto it = trackBeats->iteratorFrom(startPosition);
            it != trackBeats->cend() && *it <= endPosition;
            ++it) {
        if (it.isMarker() && m_displayMarkerbeat) {
            numMarkerbeatsInRange++;
        } else if (it.isDownbeat() && m_displayDownbeat) {
            numDownbeatsInRange++;
        } else {
            numBeatsInRange++;
        }
    }

    const int reservedBeat = numBeatsInRange * numVerticesPerLine,
              reservedDownbeat = numDownbeatsInRange * numVerticesPerLine,
              reservedMarkerbeat = numMarkerbeatsInRange * numVerticesPerLine;
    m_beatVertices.clear();
    m_downbeatVertices.clear();
    m_markerbeatVertices.clear();
    m_beatVertices.reserve(reservedBeat);
    m_downbeatVertices.reserve(reservedDownbeat);
    m_markerbeatVertices.reserve(reservedMarkerbeat);

    for (auto it = trackBeats->iteratorFrom(startPosition);
            it != trackBeats->cend() && *it <= endPosition;
            ++it) {
        double beatPosition = it->toEngineSamplePos();
        double xBeatPoint =
                m_waveformRenderer->transformSamplePositionInRendererWorld(
                        beatPosition, positionType);

        xBeatPoint = qRound(xBeatPoint * devicePixelRatio) / devicePixelRatio;

        const float x1 = static_cast<float>(xBeatPoint);
        const float x2 = x1 + 1.f;

        auto& vertices = it.isMarker() && m_displayMarkerbeat
                ? m_markerbeatVertices
                : (it.isDownbeat() && m_displayDownbeat ? m_downbeatVertices : m_beatVertices);
        vertices.addRectangle(x1,
                0.f,
                x2,
                m_isSlipRenderer ? rendererBreadth / 2 : rendererBreadth);
    }

    DEBUG_ASSERT(reservedBeat == m_beatVertices.size());
    DEBUG_ASSERT(reservedDownbeat == m_downbeatVertices.size());
    DEBUG_ASSERT(reservedMarkerbeat == m_markerbeatVertices.size());
    {
        // Draw the regular beat grid
        const int positionLocation = m_beatShader.positionLocation();
        const int matrixLocation = m_beatShader.matrixLocation();
        const int colorLocation = m_beatShader.colorLocation();

        m_beatShader.bind();
        m_beatShader.enableAttributeArray(positionLocation);

        const QMatrix4x4 matrix = matrixForWidgetGeometry(m_waveformRenderer, false);

        m_beatShader.setAttributeArray(
                positionLocation, GL_FLOAT, m_beatVertices.constData(), 2);

        m_beatShader.setUniformValue(matrixLocation, matrix);
        m_beatShader.setUniformValue(colorLocation, beatColor);

        glDrawArrays(GL_TRIANGLES, 0, m_beatVertices.size());

        m_beatShader.disableAttributeArray(positionLocation);
        m_beatShader.release();
    }

    if (m_displayDownbeat) {
        // Draw the down beat grid
        const int positionLocation = m_downbeatShader.positionLocation();
        const int matrixLocation = m_downbeatShader.matrixLocation();
        const int colorLocation = m_downbeatShader.colorLocation();

        m_downbeatShader.bind();
        m_downbeatShader.enableAttributeArray(positionLocation);

        const QMatrix4x4 matrix = matrixForWidgetGeometry(m_waveformRenderer, false);

        m_downbeatShader.setAttributeArray(
                positionLocation, GL_FLOAT, m_downbeatVertices.constData(), 2);

        m_downbeatShader.setUniformValue(matrixLocation, matrix);
        m_downbeatShader.setUniformValue(colorLocation, downbeatColor);

        glDrawArrays(GL_TRIANGLES, 0, m_downbeatVertices.size());

        m_downbeatShader.disableAttributeArray(positionLocation);
        m_downbeatShader.release();
    }
    if (m_displayMarkerbeat) {
        // Draw the marker beat grid
        const int positionLocation = m_markerbeatShader.positionLocation();
        const int matrixLocation = m_markerbeatShader.matrixLocation();
        const int colorLocation = m_markerbeatShader.colorLocation();

        m_markerbeatShader.bind();
        m_markerbeatShader.enableAttributeArray(positionLocation);

        const QMatrix4x4 matrix = matrixForWidgetGeometry(m_waveformRenderer, false);

        m_markerbeatShader.setAttributeArray(
                positionLocation, GL_FLOAT, m_markerbeatVertices.constData(), 2);

        m_markerbeatShader.setUniformValue(matrixLocation, matrix);
        m_markerbeatShader.setUniformValue(colorLocation, markerbeatColor);

        glDrawArrays(GL_TRIANGLES, 0, m_markerbeatVertices.size());

        m_markerbeatShader.disableAttributeArray(positionLocation);
        m_markerbeatShader.release();
    }
}

} // namespace allshader
