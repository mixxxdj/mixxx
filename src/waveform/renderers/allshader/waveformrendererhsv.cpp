#include "waveform/renderers/allshader/waveformrendererhsv.h"

#include "track/track.h"
#include "util/colorcomponents.h"
#include "util/math.h"
#include "waveform/renderers/allshader/matrixforwidgetgeometry.h"
#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"
#include "waveform/widgets/allshader/waveformwidget.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

namespace allshader {

WaveformRendererHSV::WaveformRendererHSV(
        WaveformWidgetRenderer* waveformWidget)
        : WaveformRendererSignalBase(waveformWidget) {
}

void WaveformRendererHSV::onSetup(const QDomNode& node) {
    Q_UNUSED(node);
}

void WaveformRendererHSV::initializeGL() {
    WaveformRendererSignalBase::initializeGL();
    m_shader.init();
}

void WaveformRendererHSV::paintGL() {
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

    float allGain(1.0);
    getGains(&allGain, nullptr, nullptr, nullptr);

    // Get base color of waveform in the HSV format (s and v isn't use)
    float h, s, v;
    getHsvF(m_pColors->getLowColor(), &h, &s, &v);

    const float breadth = static_cast<float>(m_waveformRenderer->getBreadth()) * devicePixelRatio;
    const float halfBreadth = breadth / 2.0f;

    const float heightFactor = allGain * halfBreadth / 256.f;

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

        // per channel
        float maxLow[2]{};
        float maxMid[2]{};
        float maxHigh[2]{};
        float maxAll[2]{};

        for (int chn = 0; chn < 2; chn++) {
            // data is interleaved left / right
            for (int i = visualIndexStart + chn; i < visualIndexStop + chn; i += 2) {
                const WaveformData& waveformData = data[i];

                const float filteredLow = static_cast<float>(waveformData.filtered.low);
                const float filteredMid = static_cast<float>(waveformData.filtered.mid);
                const float filteredHigh = static_cast<float>(waveformData.filtered.high);
                const float filteredAll = static_cast<float>(waveformData.filtered.all);

                maxLow[chn] = math_max(maxLow[chn], filteredLow);
                maxMid[chn] = math_max(maxMid[chn], filteredMid);
                maxHigh[chn] = math_max(maxHigh[chn], filteredHigh);
                maxAll[chn] = math_max(maxAll[chn], filteredAll);
            }
        }

        float total{};
        float lo{};
        float hi{};

        if (maxAll[0] != 0.f && maxAll[1] != 0.f) {
            // Calculate sum, to normalize
            // Also multiply on 1.2 to prevent very dark or light color
            total = (maxLow[0] + maxLow[1] + maxMid[0] + maxMid[1] +
                            maxHigh[0] + maxHigh[1]) *
                    1.2f;

            // prevent division by zero
            if (total != 0.f) {
                // Normalize low and high (mid not need, because it not change the color)
                lo = (maxLow[0] + maxLow[1]) / total;
                hi = (maxHigh[0] + maxHigh[1]) / total;
            }
        }

        // Set color
        QColor color;
        color.setHsvF(h, 1.0f - hi, 1.0f - lo);

        // lines are thin rectangles
        // maxAll[0] is for left channel, maxAll[1] is for right channel
        m_vertices.addRectangle(fpos - 0.5f,
                halfBreadth - heightFactor * maxAll[0],
                fpos + 0.5f,
                halfBreadth + heightFactor * maxAll[1]);
        m_colors.addForRectangle(
                static_cast<float>(color.redF()),
                static_cast<float>(color.greenF()),
                static_cast<float>(color.blueF()));

        xVisualSampleIndex += visualIncrementPerPixel;
    }

    DEBUG_ASSERT(reserved == m_vertices.size());
    DEBUG_ASSERT(reserved == m_colors.size());

    const QMatrix4x4 matrix = matrixForWidgetGeometry(m_waveformRenderer, true);

    const int matrixLocation = m_shader.uniformLocation("matrix");
    const int positionLocation = m_shader.attributeLocation("position");
    const int colorLocation = m_shader.attributeLocation("color");

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
