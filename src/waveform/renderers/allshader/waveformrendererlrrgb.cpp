#include "waveform/renderers/allshader/waveformrendererlrrgb.h"

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

WaveformRendererLRRGB::WaveformRendererLRRGB(
        WaveformWidgetRenderer* waveformWidget)
        : WaveformRendererSignalBase(waveformWidget) {
}

void WaveformRendererLRRGB::onSetup(const QDomNode& node) {
    Q_UNUSED(node);
}

void WaveformRendererLRRGB::initializeGL() {
    WaveformRendererSignalBase::initializeGL();
    m_shader.init();
}

void WaveformRendererLRRGB::paintGL() {
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

    const float heightFactorAbs = allGain * halfBreadth / m_maxValue;
    const float heightFactor[2] = {-heightFactorAbs, heightFactorAbs};

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
    double xVisualFrame = qRound(firstVisualFrame / visualIncrementPerPixel) *
            visualIncrementPerPixel;

    const int numVerticesPerLine = 6; // 2 triangles

    const int reserved = numVerticesPerLine * (length * 2 + 1);

    m_vertices.clear();
    m_vertices.reserve(reserved);
    m_colors.clear();
    m_colors.reserve(reserved);

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

        for (int chn = 0; chn < 2; chn++) {
            // Find the max values for low, mid, high and all in the waveform data
            uchar u8maxLow{};
            uchar u8maxMid{};
            uchar u8maxHigh{};
            uchar u8maxAll{};
            // data is interleaved left / right
            for (int i = visualIndexStart + chn; i < visualIndexStop + chn; i += 2) {
                const WaveformData& waveformData = data[i];

                u8maxLow = math_max(u8maxLow, waveformData.filtered.low);
                u8maxMid = math_max(u8maxMid, waveformData.filtered.mid);
                u8maxHigh = math_max(u8maxHigh, waveformData.filtered.high);
                u8maxAll = math_max(u8maxAll, waveformData.filtered.all);
            }

            // Cast to float
            float maxLow = static_cast<float>(u8maxLow);
            float maxMid = static_cast<float>(u8maxMid);
            float maxHigh = static_cast<float>(u8maxHigh);
            float maxAll = static_cast<float>(u8maxAll);
            // Uncomment to undo scaling with pow(value, 2.0f * 0.316f) done in analyzerwaveform.h
            // float maxAll = unscale(u8maxAll);

            // Calculate the squared magnitude of the maxLow, maxMid and maxHigh values.
            // We take the square root to get the magnitude below.
            const float sum = math_pow2(maxLow) + math_pow2(maxMid) + math_pow2(maxHigh);

            // Apply the gains
            maxLow *= lowGain;
            maxMid *= midGain;
            maxHigh *= highGain;

            // Calculate the squared magnitude of the gained maxLow, maxMid and maxHigh values
            // We take the square root to get the magnitude below.
            const float sumGained = math_pow2(maxLow) + math_pow2(maxMid) + math_pow2(maxHigh);

            // The maxAll value will be used to draw the amplitude. We scale them according to
            // magnitude of the gained maxLow, maxMid and maxHigh values
            if (sum != 0.f) {
                // magnitude = sqrt(sum) and magnitudeGained = sqrt(sumGained), and
                // factor = magnitudeGained / magnitude, but we can do with a single sqrt:
                const float factor = std::sqrt(sumGained / sum);
                maxAll *= factor;
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

            // lines are thin rectangles
            // note: heightFactor is the same for left and right,
            // but negative for left (chn 0) and positive for right (chn 1)
            m_vertices.addRectangle(fpos - 0.5f,
                    halfBreadth,
                    fpos + 0.5f,
                    halfBreadth + heightFactor[chn] * maxAll);
            m_colors.addForRectangle(red, green, blue);
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
            colorLocation, GL_FLOAT, m_colors.constData(), 3);

    glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());

    m_shader.disableAttributeArray(positionLocation);
    m_shader.disableAttributeArray(colorLocation);
    m_shader.release();
}

} // namespace allshader
