#include "waveform/renderers/qopengl/waveformrendererlrrgb.h"

#include <iostream>

#include "track/track.h"
#include "util/math.h"
#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"
#include "waveform/widgets/qopengl/waveformwidget.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

using namespace qopengl;

namespace {
inline float math_pow2(float x) {
    return x * x;
}
} // namespace

WaveformRendererLRRGB::WaveformRendererLRRGB(
        WaveformWidgetRenderer* waveformWidget)
        : WaveformRendererSignalBase(waveformWidget) {
}

WaveformRendererLRRGB::~WaveformRendererLRRGB() {
}

void WaveformRendererLRRGB::onSetup(const QDomNode& node) {
    Q_UNUSED(node);
}

void WaveformRendererLRRGB::initializeGL() {
    QString vertexShaderCode = QStringLiteral(R"--(
uniform mat4 matrix;
attribute vec4 position;
attribute vec3 color;
varying vec3 vcolor;
void main()
{
    vcolor = color;
    gl_Position = matrix * position;
}
)--");

    QString fragmentShaderCode = QStringLiteral(R"--(
varying vec3 vcolor;
void main()
{
    gl_FragColor = vec4(vcolor,1.0);
}
)--");

    initShaders(vertexShaderCode, fragmentShaderCode);
}

void WaveformRendererLRRGB::addRectangle(
        float x1,
        float y1,
        float x2,
        float y2,
        float r,
        float g,
        float b) {
    m_vertices.push_back({x1, y1});
    m_vertices.push_back({x2, y1});
    m_vertices.push_back({x1, y2});
    m_vertices.push_back({x1, y2});
    m_vertices.push_back({x2, y2});
    m_vertices.push_back({x2, y1});

    for (int i = 0; i < 6; i++) {
        m_colors.push_back({r, g, b});
    }
}

void WaveformRendererLRRGB::renderGL() {
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
    const double gain = (lastVisualIndex - firstVisualIndex) / static_cast<double>(length);

    // Per-band gain from the EQ knobs.
    float allGain(1.0), lowGain(1.0), midGain(1.0), highGain(1.0);
    getGains(&allGain, &lowGain, &midGain, &highGain);

    const float breadth = static_cast<float>(m_waveformRenderer->getBreadth()) * devicePixelRatio;
    const float halfBreadth = breadth / 2.0f;

    const float heightFactorAbs = allGain * halfBreadth / std::sqrt(3.f * 256.f * 256.f);
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

    // Effective visual index of x
    double xVisualSampleIndex = firstVisualIndex;

    const int numVerticesPerLine = 6; // 2 triangles

    const int linesReserved = numVerticesPerLine * (length * 2 + 1);
    const int colorsReserved = numVerticesPerLine * (length * 2 + 1);

    m_vertices.clear();
    m_vertices.reserve(linesReserved);
    m_colors.clear();
    m_colors.reserve(colorsReserved);

    addRectangle(0.f,
            halfBreadth - 0.5f * devicePixelRatio,
            static_cast<float>(length),
            halfBreadth + 0.5f * devicePixelRatio,
            1.f,
            1.f,
            1.f);

    for (int pos = 0; pos < length; ++pos) {
        // Our current pixel (x) corresponds to a number of visual samples
        // (visualSamplerPerPixel) in our waveform object. We take the max of
        // all the data points on either side of xVisualSampleIndex within a
        // window of 'maxSamplingRange' visual samples to measure the maximum
        // data point contained by this pixel.
        double maxSamplingRange = gain / 2.0;

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

        for (int chn = 0; chn < 2; chn++) {
            float maxLow{};
            float maxMid{};
            float maxHigh{};
            float maxAll{};

            // data is interleaved left / right
            for (int i = visualIndexStart + chn; i < visualIndexStop + chn; i += 2) {
                const WaveformData& waveformData = data[i];

                const float filteredLow = static_cast<float>(waveformData.filtered.low);
                const float filteredMid = static_cast<float>(waveformData.filtered.mid);
                const float filteredHigh = static_cast<float>(waveformData.filtered.high);

                maxLow = math_max(maxLow, filteredLow);
                maxMid = math_max(maxMid, filteredMid);
                maxHigh = math_max(maxHigh, filteredHigh);

                const float all = math_pow2(filteredLow) * lowGain +
                        math_pow2(filteredMid) * midGain +
                        math_pow2(filteredHigh) * highGain;
                maxAll = math_max(maxAll, all);
            }

            maxLow *= lowGain;
            maxMid *= midGain;
            maxHigh *= highGain;

            float red = maxLow * low_r + maxMid * mid_r + maxHigh * high_r;
            float green = maxLow * low_g + maxMid * mid_g + maxHigh * high_g;
            float blue = maxLow * low_b + maxMid * mid_b + maxHigh * high_b;

            const float max = math_max3(red, green, blue);

            // Normalize red, green, blue, using the maximum of the three

            if (max == 0.f) {
                // avoid division by 0
                red = 0.f;
                green = 0.f;
                blue = 0.f;
            } else {
                const float normFactor = 1.f / max;
                red *= normFactor;
                green *= normFactor;
                blue *= normFactor;
            }

            // lines are thin rectangles
            // note: heightFactor is the same for left and right,
            // but negative for left (chn 0) and positive for right (chn 1)
            addRectangle(fpos - 0.5f,
                    halfBreadth,
                    fpos + 0.5f,
                    halfBreadth + heightFactor[chn] * std::sqrt(maxAll),
                    red,
                    green,
                    blue);
        }

        xVisualSampleIndex += gain;
    }

    DEBUG_ASSERT(linesReserved == m_vertices.size());
    DEBUG_ASSERT(colorsReserved == m_colors.size());

    QMatrix4x4 matrix;
    matrix.ortho(QRectF(0.0,
            0.0,
            m_waveformRenderer->getWidth() * devicePixelRatio,
            m_waveformRenderer->getHeight() * devicePixelRatio));
    if (m_waveformRenderer->getOrientation() == Qt::Vertical) {
        matrix.rotate(90.f, 0.0f, 0.0f, 1.0f);
        matrix.translate(0.f, -m_waveformRenderer->getWidth() * devicePixelRatio, 0.f);
    }

    const int matrixLocation = m_shaderProgram.uniformLocation("matrix");
    const int positionLocation = m_shaderProgram.attributeLocation("position");
    const int colorLocation = m_shaderProgram.attributeLocation("color");

    m_shaderProgram.bind();
    m_shaderProgram.enableAttributeArray(positionLocation);
    m_shaderProgram.enableAttributeArray(colorLocation);

    m_shaderProgram.setUniformValue(matrixLocation, matrix);

    m_shaderProgram.setAttributeArray(
            positionLocation, GL_FLOAT, m_vertices.constData(), 2);
    m_shaderProgram.setAttributeArray(
            colorLocation, GL_FLOAT, m_colors.constData(), 3);

    glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());

    m_shaderProgram.disableAttributeArray(positionLocation);
    m_shaderProgram.disableAttributeArray(colorLocation);
    m_shaderProgram.release();
}
