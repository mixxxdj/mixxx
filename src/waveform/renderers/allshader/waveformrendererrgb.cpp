#include "waveform/renderers/allshader/waveformrendererrgb.h"

#include "track/track.h"
#include "util/math.h"
#include "waveform/renderers/allshader/matrixforwidgetgeometry.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveform.h"

namespace allshader {

namespace {
inline float math_pow2(float x) {
    return x * x;
}
} // namespace

WaveformRendererRGB::WaveformRendererRGB(
        WaveformWidgetRenderer* waveformWidget)
        : WaveformRendererSignalBase(waveformWidget) {
}

void WaveformRendererRGB::onSetup(const QDomNode& node) {
    Q_UNUSED(node);
}

void WaveformRendererRGB::initializeGL() {
    WaveformRendererSignalBase::initializeGL();
    m_shader.init();
    m_shader_ghost.init();
}

void WaveformRendererRGB::paintGL() {
    TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    if (!pTrack) {
        return;
    }

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

    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();
    const int length = static_cast<int>(m_waveformRenderer->getLength() * devicePixelRatio);

    // See waveformrenderersimple.cpp for a detailed explanation of the frame and index calculation
    const int visualFramesSize = dataSize / 2;
    const double firstVisualFrame =
            m_waveformRenderer->getFirstDisplayedPosition() * visualFramesSize;
    const double lastVisualFrame =
            m_waveformRenderer->getLastDisplayedPosition() * visualFramesSize;

    // Represents the # of visual frames per horizontal pixel.
    const double visualIncrementPerPixel =
            (lastVisualFrame - firstVisualFrame) / static_cast<double>(length);

    // Per-band gain from the EQ knobs.
    float allGain(1.0), lowGain(1.0), midGain(1.0), highGain(1.0);
    // applyCompensation = false, as we scale to match filtered.all
    getGains(&allGain, false, &lowGain, &midGain, &highGain);

    const float breadth = static_cast<float>(m_waveformRenderer->getBreadth()) * devicePixelRatio;
    const float halfBreadth = breadth / 2.0f;

    const float heightFactor = allGain * halfBreadth / m_maxValue;

    const float low_r = static_cast<float>(m_rgbLowColor_r);
    const float mid_r = static_cast<float>(m_rgbMidColor_r);
    const float high_r = static_cast<float>(m_rgbHighColor_r);
    const float low_g = static_cast<float>(m_rgbLowColor_g);
    const float mid_g = static_cast<float>(m_rgbMidColor_g);
    const float high_g = static_cast<float>(m_rgbHighColor_g);
    const float low_b = static_cast<float>(m_rgbLowColor_b);
    const float mid_b = static_cast<float>(m_rgbMidColor_b);
    const float high_b = static_cast<float>(m_rgbHighColor_b);

    // Effective visual frame for x
    double xVisualFrame = firstVisualFrame;

    const int numVerticesPerLine = 6; // 2 triangles

    const int reserved = numVerticesPerLine * (length + 1);

    m_vertices.clear();
    m_vertices.reserve(reserved);
    m_colors.clear();
    m_colors.reserve(reserved);
    m_vertices_ghost.clear();
    m_vertices_ghost.reserve(reserved);
    m_colors_ghost.clear();
    m_colors_ghost.reserve(reserved);

    m_vertices.addRectangle(0.f,
            halfBreadth - 0.5f * devicePixelRatio,
            static_cast<float>(length),
            halfBreadth + 0.5f * devicePixelRatio);
    m_colors.addForRectangle(
            static_cast<float>(m_axesColor_r),
            static_cast<float>(m_axesColor_g),
            static_cast<float>(m_axesColor_b));

    const double maxSamplingRange = visualIncrementPerPixel / 2.0;

    for (int pos = 0; pos < length; ++pos) {
        const int visualFrameStart = std::lround(xVisualFrame - maxSamplingRange);
        const int visualFrameStop = std::lround(xVisualFrame + maxSamplingRange);

        const int visualIndexStart = std::max(visualFrameStart * 2, 0);
        const int visualIndexStop =
                std::min(std::max(visualFrameStop, visualFrameStart + 1) * 2, dataSize - 1);

        const float fpos = static_cast<float>(pos);

        // Find the max values for low, mid, high and all in the waveform data.
        // - Max of left and right
        uchar u8maxLow{};
        uchar u8maxMid{};
        uchar u8maxHigh{};
        // - Per channel
        uchar u8maxAllChn[2]{};
        for (int chn = 0; chn < 2; chn++) {
            // data is interleaved left / right
            for (int i = visualIndexStart + chn; i < visualIndexStop + chn; i += 2) {
                const WaveformData& waveformData = data[i];

                u8maxLow = math_max(u8maxLow, waveformData.filtered.low);
                u8maxMid = math_max(u8maxMid, waveformData.filtered.mid);
                u8maxHigh = math_max(u8maxHigh, waveformData.filtered.high);
                u8maxAllChn[chn] = math_max(u8maxAllChn[chn], waveformData.filtered.all);
            }
        }

        // Cast to float
        float maxLow = static_cast<float>(u8maxLow);
        float maxMid = static_cast<float>(u8maxMid);
        float maxHigh = static_cast<float>(u8maxHigh);
        float maxAllChn[2]{static_cast<float>(u8maxAllChn[0]), static_cast<float>(u8maxAllChn[1])};

        float ghostLow = maxLow;
        float ghostMid = maxMid;
        float ghostHigh = maxHigh;
        float maxAllChn_ghost[2]{static_cast<float>(u8maxAllChn[0]),
                static_cast<float>(u8maxAllChn[1])};
        // Uncomment to undo scaling with pow(value, 2.0f * 0.316f) done in analyzerwaveform.h
        // float maxAllChn[2]{unscale(u8maxAllChn[0]), unscale(u8maxAllChn[1])};

        // Calculate the squared magnitude of the maxLow, maxMid and maxHigh values.
        // We take the square root to get the magnitude below.
        const float sum = math_pow2(maxLow) + math_pow2(maxMid) + math_pow2(maxHigh);

        // Apply the gains (non-ghosted version only)
        maxLow *= lowGain;
        maxMid *= midGain;
        maxHigh *= highGain;

        // Calculate the squared magnitude of the gained maxLow, maxMid and maxHigh values
        // We take the square root to get the magnitude below.
        const float sumGained = math_pow2(maxLow) + math_pow2(maxMid) + math_pow2(maxHigh);
        const float sumGained_ghost = math_pow2(ghostLow) +
                math_pow2(ghostMid) + math_pow2(ghostHigh);

        // The maxAll values will be used to draw the amplitude. We scale them according to
        // magnitude of the gained maxLow, maxMid and maxHigh values
        if (sum != 0.f) {
            // magnitude = sqrt(sum) and magnitudeGained = sqrt(sumGained), and
            // factor = magnitudeGained / magnitude, but we can do with a single sqrt:
            const float factor = std::sqrt(sumGained / sum);
            maxAllChn[0] *= factor;
            maxAllChn[1] *= factor;

            const float factor_ghost = std::sqrt(sumGained_ghost / sum);
            maxAllChn_ghost[0] *= factor_ghost;
            maxAllChn_ghost[1] *= factor_ghost;
        }

        // Use the gained maxLow, maxMid and maxHigh values to calculate the color components
        float red = maxLow * low_r + maxMid * mid_r + maxHigh * high_r;
        float green = maxLow * low_g + maxMid * mid_g + maxHigh * high_g;
        float blue = maxLow * low_b + maxMid * mid_b + maxHigh * high_b;

        // Normalize the color components using the maximum of the three
        const float maxComponent = math_max3(red, green, blue);
        if (maxComponent == 0.f) {
            // Avoid division by 0
            red = 0.f;
            green = 0.f;
            blue = 0.f;
        } else {
            const float normFactor = 1.f / maxComponent;
            red *= normFactor;
            green *= normFactor;
            blue *= normFactor;
        }

        float red_ghost = ghostLow * low_r + ghostMid * mid_r + ghostHigh * high_r;
        float green_ghost = ghostLow * low_g + ghostMid * mid_g + ghostHigh * high_g;
        float blue_ghost = ghostLow * low_b + ghostMid * mid_b + ghostHigh * high_b;
        const float maxComponent_ghost = math_max3(red_ghost, green_ghost, blue_ghost);
        if (maxComponent_ghost == 0.f) {
            // Avoid division by 0
            red_ghost = 0.f;
            green_ghost = 0.f;
            blue_ghost = 0.f;
        } else {
            const float normFactor_ghost = 1.f / maxComponent_ghost;
            red_ghost *= normFactor_ghost;
            green_ghost *= normFactor_ghost;
            blue_ghost *= normFactor_ghost;
        }

        // Lines are thin rectangles
        m_vertices.addRectangle(fpos - 0.5f,
                halfBreadth - heightFactor * maxAllChn[0],
                fpos + 0.5f,
                halfBreadth + heightFactor * maxAllChn[1]);
        m_colors.addForRectangle(red, green, blue);

        m_vertices_ghost.addRectangle(fpos - 0.5f,
                halfBreadth - heightFactor * maxAllChn_ghost[0],
                fpos + 0.5f,
                halfBreadth + heightFactor * maxAllChn_ghost[1]);
        m_colors_ghost.addForRectangle(red_ghost, green_ghost, blue_ghost, ghost_alpha);

        xVisualFrame += visualIncrementPerPixel;
    }

    DEBUG_ASSERT(reserved == m_vertices.size());
    DEBUG_ASSERT(reserved == m_colors.size());

    const QMatrix4x4 matrix = matrixForWidgetGeometry(m_waveformRenderer, true);

    // Draw the ghost version first so the solid version is on top.
    const int matrixLocation_ghost = m_shader_ghost.matrixLocation();
    const int positionLocation_ghost = m_shader_ghost.positionLocation();
    const int colorLocation_ghost = m_shader_ghost.colorLocation();

    m_shader_ghost.bind();
    m_shader_ghost.enableAttributeArray(positionLocation_ghost);
    m_shader_ghost.enableAttributeArray(colorLocation_ghost);

    m_shader_ghost.setUniformValue(matrixLocation_ghost, matrix);

    m_shader_ghost.setAttributeArray(
            positionLocation_ghost, GL_FLOAT, m_vertices_ghost.constData(), 2);
    m_shader_ghost.setAttributeArray(
            colorLocation_ghost, GL_FLOAT, m_colors_ghost.constData(), 4);

    glDrawArrays(GL_TRIANGLES, 0, m_vertices_ghost.size());

    m_shader_ghost.disableAttributeArray(positionLocation_ghost);
    m_shader_ghost.disableAttributeArray(colorLocation_ghost);
    m_shader_ghost.release();

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
            colorLocation, GL_FLOAT, m_colors.constData(), 3);

    glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());

    m_shader.disableAttributeArray(positionLocation);
    m_shader.disableAttributeArray(colorLocation);
    m_shader.release();
}

} // namespace allshader
