#include "waveform/renderers/allshader/waveformrendererstem.h"

#include <QFont>
#include <QImage>
#include <QOpenGLTexture>

#include "engine/engine.h"
#include "track/track.h"
#include "util/math.h"
#include "waveform/renderers/allshader/matrixforwidgetgeometry.h"
#include "waveform/renderers/allshader/rgbdata.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveform.h"

namespace allshader {

WaveformRendererStem::WaveformRendererStem(
        WaveformWidgetRenderer* waveformWidget,
        ::WaveformRendererAbstract::PositionSource type)
        : WaveformRendererSignalBase(waveformWidget),
          m_isSlipRenderer(type == ::WaveformRendererAbstract::Slip) {
}

void WaveformRendererStem::onSetup(const QDomNode& node) {
    Q_UNUSED(node);
}

void WaveformRendererStem::initializeGL() {
    WaveformRendererSignalBase::initializeGL();
    m_shader.init();
    m_textureShader.init();
    auto group = m_pEQEnabled->getKey().group;
    for (int stemIdx = 1; stemIdx <= mixxx::kMaxSupportedStems; stemIdx++) {
        DEBUG_ASSERT(group.endsWith("]"));
        QString stemGroup = QStringLiteral("%1Stem%2]")
                                    .arg(group.left(group.size() - 1),
                                            QString::number(stemIdx));
        m_pStemGain.emplace_back(
                std::make_unique<ControlProxy>(stemGroup,
                        QStringLiteral("volume")));
        m_pStemMute.emplace_back(std::make_unique<ControlProxy>(
                stemGroup, QStringLiteral("mute")));
    }
}

void WaveformRendererStem::paintGL() {
    TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    if (!pTrack || (m_isSlipRenderer && !m_waveformRenderer->isSlipActive())) {
        return;
    }

    auto stemInfo = pTrack->getStemInfo();
    // If this track isn't a stem track, skip the rendering
    if (stemInfo.isEmpty()) {
        return;
    }
    auto positionType = m_isSlipRenderer ? ::WaveformRendererAbstract::Slip
                                         : ::WaveformRendererAbstract::Play;

    ConstWaveformPointer waveform = pTrack->getWaveform();
    if (waveform.isNull()) {
        return;
    }

    const int dataSize = waveform->getDataSize();
    if (dataSize <= 1) {
        return;
    }

    const WaveformData* data = waveform->data();
    if (data == nullptr) {
        return;
    }
    // If this waveform doesn't contain stem data, skip the rendering
    if (!waveform->hasStem()) {
        return;
    }

    uint selectedStems = m_waveformRenderer->getSelectedStems();

    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();
    const int length = static_cast<int>(m_waveformRenderer->getLength() * devicePixelRatio);

    // See waveformrenderersimple.cpp for a detailed explanation of the frame and index calculation
    const int visualFramesSize = dataSize / 2;
    const double firstVisualFrame =
            m_waveformRenderer->getFirstDisplayedPosition(positionType) * visualFramesSize;
    const double lastVisualFrame =
            m_waveformRenderer->getLastDisplayedPosition(positionType) * visualFramesSize;

    // Represents the # of visual frames per horizontal pixel.
    const double visualIncrementPerPixel =
            (lastVisualFrame - firstVisualFrame) / static_cast<double>(length);

    // Per-band gain from the EQ knobs.
    float allGain(1.0);
    // applyCompensation = true, as we scale to match filtered.all
    getGains(&allGain, false, nullptr, nullptr, nullptr);

    const float breadth = static_cast<float>(m_waveformRenderer->getBreadth()) * devicePixelRatio;
    const float halfBreadth = breadth / 2.0f;

    const float heightFactor = allGain * halfBreadth / m_maxValue;

    // Effective visual frame for x
    double xVisualFrame = qRound(firstVisualFrame / visualIncrementPerPixel) *
            visualIncrementPerPixel;

    const int numVerticesPerLine = 6; // 2 triangles

    const int reserved = numVerticesPerLine * (8 * length + 1);

    m_vertices.clear();
    m_vertices.reserve(reserved);
    m_colors.clear();
    m_colors.reserve(reserved);

    m_vertices.addRectangle(0.f,
            halfBreadth - 0.5f * devicePixelRatio,
            static_cast<float>(length),
            m_isSlipRenderer ? halfBreadth : halfBreadth + 0.5f * devicePixelRatio);
    m_colors.addForRectangle(0.f, 0.f, 0.f, 0.f);

    const double maxSamplingRange = visualIncrementPerPixel / 2.0;

    for (int visualIdx = 0; visualIdx < length; ++visualIdx) {
        for (int stemIdx = 0; stemIdx < mixxx::kMaxSupportedStems; stemIdx++) {
            // Stem is drawn twice with different opacity level, this allow to
            // see the maximum signal by transparency
            for (int layerIdx = 0; layerIdx < 2; layerIdx++) {
                QColor stemColor = stemInfo[stemIdx].getColor();
                float color_r = stemColor.redF(),
                      color_g = stemColor.greenF(),
                      color_b = stemColor.blueF(),
                      color_a = stemColor.alphaF() * (layerIdx ? 0.75f : 0.15f);
                const int visualFrameStart = std::lround(xVisualFrame - maxSamplingRange);
                const int visualFrameStop = std::lround(xVisualFrame + maxSamplingRange);

                const int visualIndexStart = std::max(visualFrameStart * 2, 0);
                const int visualIndexStop =
                        std::min(std::max(visualFrameStop, visualFrameStart + 1) * 2, dataSize - 1);

                const float fVisualIdx = static_cast<float>(visualIdx);

                // Find the max values for current eq in the waveform data.
                // - Max of left and right
                uchar u8max{};
                for (int chn = 0; chn < 2; chn++) {
                    // data is interleaved left / right
                    for (int i = visualIndexStart + chn; i < visualIndexStop + chn; i += 2) {
                        const WaveformData& waveformData = data[i];

                        u8max = math_max(u8max, waveformData.stems[stemIdx]);
                    }
                }

                // Cast to float
                float max = static_cast<float>(u8max);

                // Apply the gains
                if (layerIdx) {
                    max *= m_pStemMute[stemIdx]->toBool() ||
                                    (selectedStems &&
                                            !(selectedStems & 1 << stemIdx))
                            ? 0.f
                            : static_cast<float>(m_pStemGain[stemIdx]->get());
                }

                // Lines are thin rectangles
                // shawdow
                m_vertices.addRectangle(fVisualIdx - 0.5f,
                        halfBreadth - heightFactor * max,
                        fVisualIdx + 0.5f,
                        m_isSlipRenderer ? halfBreadth : halfBreadth + heightFactor * max);

                m_colors.addForRectangle(color_r, color_g, color_b, color_a);
            }
        }
        xVisualFrame += visualIncrementPerPixel;
    }

    DEBUG_ASSERT(reserved == m_vertices.size());
    DEBUG_ASSERT(reserved == m_colors.size());

    const QMatrix4x4 matrix = matrixForWidgetGeometry(m_waveformRenderer, true);

    const int matrixLocation = m_shader.matrixLocation();
    const int positionLocation = m_shader.positionLocation();
    const int colorLocation = m_shader.colorLocation();

    m_shader.bind();
    m_shader.enableAttributeArray(positionLocation);
    m_shader.enableAttributeArray(colorLocation);

    m_shader.setUniformValue(matrixLocation, matrix);

    m_shader.setAttributeArray(
            positionLocation, GL_FLOAT, m_vertices.constData(), 2);
    m_shader.setAttributeArray(
            colorLocation, GL_FLOAT, m_colors.constData(), 4);

    glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());

    m_shader.disableAttributeArray(positionLocation);
    m_shader.disableAttributeArray(colorLocation);
    m_shader.release();
}

} // namespace allshader
