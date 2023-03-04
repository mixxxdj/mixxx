#include "waveform/renderers/qopengl/waveformrendererrgb.h"

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

WaveformRendererRGB::WaveformRendererRGB(
        WaveformWidgetRenderer* waveformWidget)
        : WaveformRendererSignalBase(waveformWidget) {
}

WaveformRendererRGB::~WaveformRendererRGB() {
}

void WaveformRendererRGB::onSetup(const QDomNode& node) {
    Q_UNUSED(node);
}

void WaveformRendererRGB::initializeGL() {
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

inline void WaveformRendererRGB::addRectangle(
        float x1,
        float y1,
        float x2,
        float y2,
        float r,
        float g,
        float b) {
    m_lines[m_lineIndex++] = x1;
    m_lines[m_lineIndex++] = y1;

    m_lines[m_lineIndex++] = x2;
    m_lines[m_lineIndex++] = y1;

    m_lines[m_lineIndex++] = x1;
    m_lines[m_lineIndex++] = y2;

    m_lines[m_lineIndex++] = x1;
    m_lines[m_lineIndex++] = y2;

    m_lines[m_lineIndex++] = x2;
    m_lines[m_lineIndex++] = y2;

    m_lines[m_lineIndex++] = x2;
    m_lines[m_lineIndex++] = y1;

    for (int i = 0; i < 6; i++) {
        m_colors[m_colorIndex++] = r;
        m_colors[m_colorIndex++] = g;
        m_colors[m_colorIndex++] = b;
    }
}

void WaveformRendererRGB::renderGL() {
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
    const int n = static_cast<int>(static_cast<float>(
            m_waveformRenderer->getLength() * devicePixelRatio));

    // Not multiplying with devicePixelRatio will also work, and on retina
    // displays 2 pixels will be used to represent each block of samples. This
    // is also what is done for the beat grid and the markers. const int n =
    // static_cast<int>(static_cast<float>(m_waveformRenderer->getLength()));

    const double firstVisualIndex = m_waveformRenderer->getFirstDisplayedPosition() * dataSize;
    const double lastVisualIndex = m_waveformRenderer->getLastDisplayedPosition() * dataSize;

    // Represents the # of waveform data points per horizontal pixel.
    const double gain = (lastVisualIndex - firstVisualIndex) / static_cast<double>(n);

    // Per-band gain from the EQ knobs.
    float allGain(1.0), lowGain(1.0), midGain(1.0), highGain(1.0);
    getGains(&allGain, &lowGain, &midGain, &highGain);

    const float breadth = static_cast<float>(m_waveformRenderer->getBreadth());
    const float halfBreadth = breadth / 2.0f;

    const float heightFactor = allGain * halfBreadth / std::sqrt(3.f * 256.f * 256.f);

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

    m_lineIndex = 0;
    m_colorIndex = 0;

    m_lines.resize(6 * 2 * (n + 1));
    m_colors.resize(6 * 3 * (n + 1));

    addRectangle(0.f,
            halfBreadth - 0.5f,
            static_cast<float>(n),
            halfBreadth + 0.5f,
            1.f,
            1.f,
            1.f);

    for (int x = 0; x < n; ++x) {
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

        float maxLow = 0;
        float maxMid = 0;
        float maxHigh = 0;

        float maxAll = 0.;
        float maxAllNext = 0.;

        for (int i = visualIndexStart; i < visualIndexStop; i += 2) {
            const WaveformData& waveformData = data[i];
            const WaveformData& waveformDataNext = data[i + 1];

            const float filteredLow = static_cast<const float>(waveformData.filtered.low);
            const float filteredMid = static_cast<const float>(waveformData.filtered.mid);
            const float filteredHigh = static_cast<const float>(waveformData.filtered.high);

            const float nextFilteredLow = static_cast<const float>(waveformDataNext.filtered.low);
            const float nextFilteredMid = static_cast<const float>(waveformDataNext.filtered.mid);
            const float nextFilteredHigh = static_cast<const float>(waveformDataNext.filtered.high);

            maxLow = math_max3(maxLow, filteredLow, nextFilteredLow);
            maxMid = math_max3(maxMid, filteredMid, nextFilteredMid);
            maxHigh = math_max3(maxHigh, filteredHigh, nextFilteredHigh);

            const float all = math_pow2(filteredLow) * lowGain +
                    math_pow2(filteredMid) * midGain +
                    math_pow2(filteredHigh) * highGain;
            maxAll = math_max(maxAll, all);

            const float allNext = math_pow2(nextFilteredLow) * lowGain +
                    math_pow2(nextFilteredMid) * midGain +
                    math_pow2(nextFilteredHigh) * highGain;
            maxAllNext = math_max(maxAllNext, allNext);
        }

        maxLow *= lowGain;
        maxMid *= midGain;
        maxHigh *= highGain;

        float red = maxLow * low_r + maxMid * mid_r +
                maxHigh * high_r;
        float green = maxLow * low_g + maxMid * mid_g +
                maxHigh * high_g;
        float blue = maxLow * low_b + maxMid * mid_b +
                maxHigh * high_b;

        // Normalize red, green, blue, using the maximum of the three

        const float max = math_max3(red, green, blue);

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

        const float fx = static_cast<float>(x) - 0.5f;

        // lines are thin rectangles
        addRectangle(fx,
                halfBreadth - heightFactor * std::sqrt(maxAll),
                fx + 1.f,
                halfBreadth + heightFactor * std::sqrt(maxAllNext),
                red,
                green,
                blue);

        xVisualSampleIndex += gain;
    }

    QMatrix4x4 matrix;
    matrix.ortho(QRectF(0.0, 0.0, n, m_waveformRenderer->getHeight()));

    m_shaderProgram.bind();

    int matrixLocation = m_shaderProgram.uniformLocation("matrix");
    int positionLocation = m_shaderProgram.attributeLocation("position");
    int colorLocation = m_shaderProgram.attributeLocation("color");

    m_shaderProgram.setUniformValue(matrixLocation, matrix);

    m_shaderProgram.enableAttributeArray(positionLocation);
    m_shaderProgram.setAttributeArray(
            positionLocation, GL_FLOAT, m_lines.constData(), 2);
    m_shaderProgram.enableAttributeArray(colorLocation);
    m_shaderProgram.setAttributeArray(
            colorLocation, GL_FLOAT, m_colors.constData(), 3);

    glDrawArrays(GL_TRIANGLES, 0, m_lineIndex / 2);
}
