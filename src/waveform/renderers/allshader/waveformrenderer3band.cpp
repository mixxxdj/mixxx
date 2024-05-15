#include "waveform/renderers/allshader/waveformrenderer3band.h"

#include "track/track.h"
#include "util/colorcomponents.h"
#include "waveform/renderers/allshader/matrixforwidgetgeometry.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveform.h"

namespace allshader {

namespace {
// This factor defined the number of sample used for the moving average. Signal
// will look smoother but also less accurate
constexpr int kDefaultSmoothFactor = 5;
// The threshold allows the moving average to be reset in case a sample is
// significantly different from the current average value and allow to keep some
// sharpness on the waveform (e.g beat)
constexpr int kDefaultSmoothThreshold = 64.f;
constexpr int kLowIdx = 0;
constexpr int kMidIdx = 1;
constexpr int kHighIdx = 2;
// The following gains are applied on top of the existing gain (user setting and
// EQ levels) in order to prevent a complete overlapping of the signal and help
// choose a right mix of stacking and overlapping
constexpr float kLowGain = 1.3f;
constexpr float kMidGain = 0.9f;
constexpr float kHighGain = 0.4f;
} // namespace

WaveformRendererThreeBand::WaveformRendererThreeBand(
        WaveformWidgetRenderer* waveformWidget)
        : WaveformRendererSignalBase(waveformWidget),
          m_lowSmoothFactor(kDefaultSmoothFactor),
          m_midSmoothFactor(kDefaultSmoothFactor),
          m_highSmoothFactor(kDefaultSmoothFactor),
          m_lowSmoothThreshold(kDefaultSmoothThreshold),
          m_midSmoothThreshold(kDefaultSmoothThreshold),
          m_highSmoothThreshold(kDefaultSmoothThreshold) {
}

void WaveformRendererThreeBand::onSetup(const QDomNode& node) {
    bool ok;
    QDomElement lowFactor = node.firstChildElement("SmoothFactorLow");
    if (!lowFactor.isNull()) {
        int newLowFactor = lowFactor.text().toInt(&ok);
        if (ok) {
            m_lowSmoothFactor = newLowFactor;
        }
    }
    QDomElement midFactor = node.firstChildElement("SmoothFactorMid");
    if (!midFactor.isNull()) {
        int newMidFactor = midFactor.text().toInt(&ok);
        if (ok) {
            m_midSmoothFactor = newMidFactor;
        }
    }
    QDomElement highFactor = node.firstChildElement("SmoothFactorHigh");
    if (!highFactor.isNull()) {
        int newHighFactor = highFactor.text().toInt(&ok);
        if (ok) {
            m_highSmoothFactor = newHighFactor;
        }
    }
    QDomElement lowThreshold = node.firstChildElement("SmoothThresholdLow");
    if (!lowThreshold.isNull()) {
        float newLowThreshold = lowThreshold.text().toFloat(&ok);
        if (ok) {
            m_lowSmoothThreshold = newLowThreshold;
        }
    }
    QDomElement midThreshold = node.firstChildElement("SmoothThresholdMid");
    if (!midThreshold.isNull()) {
        float newMidThreshold = midThreshold.text().toFloat(&ok);
        if (ok) {
            m_midSmoothThreshold = newMidThreshold;
        }
    }
    QDomElement highThreshold = node.firstChildElement("SmoothThresholdHigh");
    if (!highThreshold.isNull()) {
        float newHighThreshold = highThreshold.text().toFloat(&ok);
        if (ok) {
            m_highSmoothThreshold = newHighThreshold;
        }
    }
    m_normalisers[kLowIdx] = MovingAverageThreshold(m_lowSmoothThreshold, m_lowSmoothFactor);
    m_normalisers[kMidIdx] = MovingAverageThreshold(m_midSmoothThreshold, m_midSmoothFactor);
    m_normalisers[kHighIdx] = MovingAverageThreshold(m_highSmoothThreshold, m_highSmoothFactor);
}

void WaveformRendererThreeBand::initializeGL() {
    WaveformRendererSignalBase::initializeGL();
    m_shader.init();
}

void WaveformRendererThreeBand::paintGL() {
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
    float allGain(1.0), gains[3]{0};
    // applyCompensation = true, as we scale to match filtered.all
    getGains(&allGain, true, gains + kLowIdx, gains + kMidIdx, gains + kHighIdx);

    gains[kLowIdx] *= kLowGain;
    gains[kMidIdx] *= kMidGain;
    gains[kHighIdx] *= kHighGain;

    const float breadth = static_cast<float>(m_waveformRenderer->getBreadth()) * devicePixelRatio;
    const float halfBreadth = breadth / 2.0f;

    const float heightFactor = allGain * halfBreadth / m_maxValue;

    // Effective visual frame for x
    double xVisualFrame = qRound(firstVisualFrame / visualIncrementPerPixel) *
            visualIncrementPerPixel;

    const int numVerticesPerLine = 6; // 2 triangles

    const int reserved = numVerticesPerLine * (3 * length + 1);

    m_vertices.clear();
    m_vertices.reserve(reserved);
    m_colors.clear();
    m_colors.reserve(reserved);

    m_vertices.addRectangle(0.f,
            halfBreadth - 0.5f * devicePixelRatio,
            static_cast<float>(length),
            halfBreadth + 0.5f * devicePixelRatio);
    m_colors.addForRectangle(0.f, 0.f, 0.f, 0.f);

    const double maxSamplingRange = visualIncrementPerPixel / 2.0;

    for (int pos = 0; pos < length; ++pos) {
        for (int eq = kLowIdx; eq < kHighIdx + 1; eq++) {
            const int visualFrameStart = std::lround(xVisualFrame - maxSamplingRange);
            const int visualFrameStop = std::lround(xVisualFrame + maxSamplingRange);

            const int visualIndexStart = std::max(visualFrameStart * 2, 0);
            const int visualIndexStop =
                    std::min(std::max(visualFrameStop, visualFrameStart + 1) * 2, dataSize - 1);

            const float fpos = static_cast<float>(pos);

            // Find the max values for current eq in the waveform data.
            // - Max of left and right
            uchar u8max{};
            for (int chn = 0; chn < 2; chn++) {
                // data is interleaved left / right
                for (int i = visualIndexStart + chn; i < visualIndexStop + chn; i += 2) {
                    const WaveformData& waveformData = data[i];

                    switch (eq) {
                    default:
                        DEBUG_ASSERT(!"Invalid EQ index");
                    case kLowIdx:
                        u8max = math_max(u8max, waveformData.filtered.low);
                        break;
                    case kMidIdx:
                        u8max = math_max(u8max, waveformData.filtered.mid);
                        break;
                    case kHighIdx:
                        u8max = math_max(u8max, waveformData.filtered.high);
                        break;
                    }
                }
            }

            // Cast to float
            float max = static_cast<float>(u8max);

            // Apply the gains
            max *= gains[eq];

            // Smooth data with a moving average
            max = m_normalisers[eq].add(max);

            // Lines are thin rectangles
            m_vertices.addRectangle(fpos - 0.5f,
                    halfBreadth - heightFactor * max,
                    fpos + 0.5f,
                    halfBreadth + heightFactor * max);
            switch (eq) {
            default:
                DEBUG_ASSERT(!"Invalid EQ index");
            case kLowIdx:
                m_colors.addForRectangle(m_lowColor_r, m_lowColor_g, m_lowColor_b, m_lowColor_a);
                break;
            case kMidIdx:
                m_colors.addForRectangle(m_midColor_r, m_midColor_g, m_midColor_b, m_midColor_a);
                break;
            case kHighIdx:
                m_colors.addForRectangle(m_highColor_r,
                        m_highColor_g,
                        m_highColor_b,
                        m_highColor_a);
                break;
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

MovingAverageThreshold::MovingAverageThreshold(float threshold, int max)
        : m_stack(),
          m_threshold(threshold),
          m_currentAverage(-1.f),
          m_cursor(0),
          m_size(0),
          m_max(max) {
    m_stack.resize(m_max);
}

void MovingAverageThreshold::reset() {
    m_currentAverage = -1.f;
    m_cursor = 0;
    m_size = 0;
}

float MovingAverageThreshold::add(float sample) {
    if (m_currentAverage > 0 && fabs(sample - m_currentAverage) > m_threshold) {
        m_cursor = 0;
        m_size = 0;
    }
    m_stack[m_cursor] = sample;
    m_size = math_min(m_size + 1, m_max);
    m_currentAverage = m_stack[m_cursor];
    for (uint8_t i = 1; i < m_size; i++) {
        m_currentAverage += m_stack[(m_cursor + m_max - i) % m_max];
    }
    m_cursor = (m_cursor + 1) % m_max;
    m_currentAverage /= m_size;
    return m_currentAverage;
}
