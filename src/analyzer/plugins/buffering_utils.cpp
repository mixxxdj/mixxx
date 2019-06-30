#include "analyzer/plugins/buffering_utils.h"

#include "util/math.h"

namespace mixxx {

bool DownmixAndOverlapHelper::initialize(size_t windowSize, size_t stepSize, WindowReadyCallback callback) {
    m_buffer.resize(windowSize);
    m_callback = callback;
    m_windowSize = windowSize;
    m_stepSize = stepSize;
    m_bufferWritePosition = 0;
    return m_windowSize > 0 && m_stepSize > 0 &&
            m_stepSize <= m_windowSize && callback;
}

bool DownmixAndOverlapHelper::processStereoSamples(const CSAMPLE* pInput, size_t inputStereoSamples) {
    const size_t numInputFrames = inputStereoSamples / 2;
    size_t inRead = 0;
    double* pDownmix = m_buffer.data();

    while (inRead < numInputFrames) {
        size_t writeAvailable = math_min(numInputFrames,
                m_windowSize - m_bufferWritePosition);

        for (size_t i = 0; i < writeAvailable; ++i) {
            // We analyze a mono downmix of the signal since we don't think
            // stereo does us any good.
            pDownmix[m_bufferWritePosition + i] = (pInput[(inRead + i) * 2] +
                                                          pInput[(inRead + i) * 2 + 1]) *
                    0.5;
        }
        m_bufferWritePosition += writeAvailable;
        inRead += writeAvailable;

        if (m_bufferWritePosition == m_windowSize) {
            bool result = m_callback(pDownmix, m_windowSize);

            // If the callback said not to continue then stop.
            if (!result) {
                return false;
            }

            // If the window size equals the step size then this will result
            // in m_bufferWritePosition == 0.
            for (size_t i = 0; i < (m_windowSize - m_stepSize); ++i) {
                pDownmix[i] = pDownmix[i + m_stepSize];
            }
            m_bufferWritePosition -= m_stepSize;
        }
    }
    return true;
}

bool DownmixAndOverlapHelper::finalize() {
    // TODO(rryan) flush support?
    return true;
}

}