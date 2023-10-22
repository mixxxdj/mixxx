#include "waveform/renderers/allshader/waveformrendererrgb.h"

#include "track/track.h"
#include "util/math.h"
#include "waveform/renderers/allshader/matrixforwidgetgeometry.h"
#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"
#include "waveform/widgets/allshader/waveformwidget.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

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

    // Not multiplying with devicePixelRatio will also work. In that case, on
    // High-DPI-Display the lines will be devicePixelRatio pixels wide (which is
    // also what is used for the beat grid and the markers), or in other words
    // each block of samples is represented by devicePixelRatio pixels (width).

    const double firstVisualIndex = m_waveformRenderer->getFirstDisplayedPosition() * dataSize;
    const double lastVisualIndex = m_waveformRenderer->getLastDisplayedPosition() * dataSize;

    // Represents the # of waveform data points per horizontal pixel.
    const double visualIncrementPerPixel =
            (lastVisualIndex - firstVisualIndex) / static_cast<double>(length);

    // Per-band gain from the EQ knobs.
    float allGain(1.0), lowGain(1.0), midGain(1.0), highGain(1.0);
    getGains(&allGain, &lowGain, &midGain, &highGain);

    const float breadth = static_cast<float>(m_waveformRenderer->getBreadth()) * devicePixelRatio;
    const float halfBreadth = breadth / 2.0f;

    const float heightFactor = allGain * halfBreadth / 255.f;

    const float low_r = static_cast<float>(m_rgbLowColor_r);
    const float mid_r = static_cast<float>(m_rgbMidColor_r);
    const float high_r = static_cast<float>(m_rgbHighColor_r);
    const float low_g = static_cast<float>(m_rgbLowColor_g);
    const float mid_g = static_cast<float>(m_rgbMidColor_g);
    const float high_g = static_cast<float>(m_rgbHighColor_g);
    const float low_b = static_cast<float>(m_rgbLowColor_b);
    const float mid_b = static_cast<float>(m_rgbMidColor_b);
    const float high_b = static_cast<float>(m_rgbHighColor_b);

    // Effective visual index of x
    double xVisualSampleIndex = firstVisualIndex;

    const int numVerticesPerLine = 6; // 2 triangles

    const int reserved = numVerticesPerLine * (length + 1);

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

    for (int pos = 0; pos < length; ++pos) {
        // Our current pixel (x) corresponds to a number of visual samples
        // (visualSamplerPerPixel) in our waveform object. We take the max of
        // all the data points on either side of xVisualSampleIndex within a
        // window of 'maxSamplingRange' visual samples to measure the maximum
        // data point contained by this pixel.
        double maxSamplingRange = visualIncrementPerPixel / 2.0;

        // Since xVisualSampleIndex is in visual-samples (e.g. R,L,R,L) we want
        // to check +/- maxSamplingRange frames, not samples. To do this, divide
        // xVisualSampleIndex by 2. Since frames indices are integers, we round
        // to the nearest integer by adding 0.5 before casting to int.
        int visualFrameStart = int(xVisualSampleIndex / 2.0 - maxSamplingRange + 0.5);
        int visualFrameStop = int(xVisualSampleIndex / 2.0 + maxSamplingRange + 0.5);
        const int lastVisualFrame = dataSize / 2 - 1;

        // We now know that some subset of [visualFrameStart, visualFrameStop]
        // lies within the valid range of visual frames. Clamp
        // visualFrameStart/Stop to within [0, lastVisualFrame].
        visualFrameStart = math_clamp(visualFrameStart, 0, lastVisualFrame);
        visualFrameStop = math_clamp(visualFrameStop, 0, lastVisualFrame);

        int visualIndexStart = visualFrameStart * 2;
        int visualIndexStop = visualFrameStop * 2;

        visualIndexStart = std::max(visualIndexStart, 0);
        visualIndexStop = std::min(visualIndexStop, dataSize);

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

        // Calculate the magnitude of the maxLow, maxMid and maxHigh values
        const float magnitude = std::sqrt(
                math_pow2(maxLow) + math_pow2(maxMid) + math_pow2(maxHigh));

        // Apply the gains
        maxLow *= lowGain;
        maxMid *= midGain;
        maxHigh *= highGain;

        // Calculate the magnitude of the gained maxLow, maxMid and maxHigh values
        const float magnitudeGained = std::sqrt(
                math_pow2(maxLow) + math_pow2(maxMid) + math_pow2(maxHigh));

        // The maxAll values will be used to draw the amplitude. We scale them according to
        // magnitude of the gained maxLow, maxMid and maxHigh values
        if (magnitude != 0.f) {
            const float factor = magnitudeGained / magnitude;
            maxAllChn[0] *= factor;
            maxAllChn[1] *= factor;
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

        // Lines are thin rectangles
        m_vertices.addRectangle(fpos - 0.5f,
                halfBreadth - heightFactor * maxAllChn[0],
                fpos + 0.5f,
                halfBreadth + heightFactor * maxAllChn[1]);
        m_colors.addForRectangle(red, green, blue);

        xVisualSampleIndex += visualIncrementPerPixel;
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
