#include "waveform/renderers/qopengl/waveformrendererrgb.h"

#include "fixedpointcalc.h"
#include "track/track.h"
#include "util/math.h"
#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"
#include "waveform/widgets/qopengl/waveformwidget.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

using namespace qopengl;

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
    // The source of our data are uint8_t values (waveformData.filtered.low/mid/high).
    // We can avoid type conversion by calculating the values needed for drawing
    // using fixed point. Since the range of the intermediate values is limited, we
    // can take advantage of this and use a lookup table instead of sqrtf.

    uint32_t rgbLowColor_r = toFrac8(m_rgbLowColor_r);
    uint32_t rgbMidColor_r = toFrac8(m_rgbMidColor_r);
    uint32_t rgbHighColor_r = toFrac8(m_rgbHighColor_r);
    uint32_t rgbLowColor_g = toFrac8(m_rgbLowColor_g);
    uint32_t rgbMidColor_g = toFrac8(m_rgbMidColor_g);
    uint32_t rgbHighColor_g = toFrac8(m_rgbHighColor_g);
    uint32_t rgbLowColor_b = toFrac8(m_rgbLowColor_b);
    uint32_t rgbMidColor_b = toFrac8(m_rgbMidColor_b);
    uint32_t rgbHighColor_b = toFrac8(m_rgbHighColor_b);

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

    // gains in 8 bit fractional fixed point
    const uint32_t frac8LowGain(toFrac8(lowGain));
    const uint32_t frac8MidGain(toFrac8(midGain));
    const uint32_t frac8HighGain(toFrac8(highGain));

    const float breadth = static_cast<float>(m_waveformRenderer->getBreadth());
    const float halfBreadth = breadth / 2.0f;

    const float heightFactor = allGain * halfBreadth;

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

        uint32_t maxLow = 0;
        uint32_t maxMid = 0;
        uint32_t maxHigh = 0;

        uint32_t maxAll = 0.;
        uint32_t maxAllNext = 0.;

        for (int i = visualIndexStart; i < visualIndexStop; i += 2) {
            const WaveformData& waveformData = data[i];
            const WaveformData& waveformDataNext = data[i + 1];

            maxLow = math_max_u32(maxLow, waveformData.filtered.low, waveformDataNext.filtered.low);
            maxMid = math_max_u32(maxMid, waveformData.filtered.mid, waveformDataNext.filtered.mid);
            maxHigh = math_max_u32(maxHigh,
                    waveformData.filtered.high,
                    waveformDataNext.filtered.high);

            uint32_t all = frac8Pow2ToFrac16(waveformData.filtered.low * frac8LowGain) +
                    frac8Pow2ToFrac16(waveformData.filtered.mid * frac8MidGain) +
                    frac8Pow2ToFrac16(waveformData.filtered.high * frac8HighGain);
            maxAll = math_max(maxAll, all);

            uint32_t allNext = frac8Pow2ToFrac16(waveformDataNext.filtered.low * frac8LowGain) +
                    frac8Pow2ToFrac16(waveformDataNext.filtered.mid * frac8MidGain) +
                    frac8Pow2ToFrac16(waveformDataNext.filtered.high * frac8HighGain);
            maxAllNext = math_max(maxAllNext, allNext);
        }

        // We can do these integer calculation safely, staying well within the
        // 32 bit range, and we will normalize below.
        maxLow *= frac8LowGain;
        maxMid *= frac8MidGain;
        maxHigh *= frac8HighGain;
        uint32_t red = maxLow * rgbLowColor_r + maxMid * rgbMidColor_r +
                maxHigh * rgbHighColor_r;
        uint32_t green = maxLow * rgbLowColor_g + maxMid * rgbMidColor_g +
                maxHigh * rgbHighColor_g;
        uint32_t blue = maxLow * rgbLowColor_b + maxMid * rgbMidColor_b +
                maxHigh * rgbHighColor_b;

        // Normalize red, green, blue to 0..255, using the maximum of the three and
        // this fixed point arithmetic trick:
        // max / ((max>>8)+1) = 0..255
        uint32_t max = math_max_u32(red, green, blue);
        max >>= 8;

        if (max == 0) {
            // avoid division by 0
            red = 0;
            green = 0;
            blue = 0;
        } else {
            max++; // important, otherwise we normalize to 256

            red /= max;
            green /= max;
            blue /= max;
        }

        const float fx = static_cast<float>(x);

        // lines are thin rectangles
        addRectangle(fx,
                halfBreadth - heightFactor * frac16_sqrt(maxAll),
                fx + 1.f,
                halfBreadth + heightFactor * frac16_sqrt(maxAllNext),
                float(red) / 255.f,
                float(green) / 255.f,
                float(blue) / 255.f);

        xVisualSampleIndex += gain;
    }

    QMatrix4x4 matrix;
    matrix.ortho(QRectF(0, 0, n, m_waveformRenderer->getHeight()));

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
