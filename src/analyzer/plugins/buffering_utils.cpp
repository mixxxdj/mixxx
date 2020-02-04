#include "analyzer/plugins/buffering_utils.h"

#include "util/math.h"
#include "util/sample.h"

#include <string.h>

namespace mixxx {

bool DownmixAndOverlapHelper::initialize(size_t windowSize, size_t stepSize, WindowReadyCallback callback) {
    m_buffer.assign(windowSize, 0.0);
    m_callback = callback;
    m_windowSize = windowSize;
    m_stepSize = stepSize;
    // make sure the first frame is centered into the fft window. This makes sure
    // that the result is significant starting fom the first step.
    m_bufferWritePosition = windowSize / 2;
    return m_windowSize > 0 && m_stepSize > 0 &&
            m_stepSize <= m_windowSize && callback;
}

bool DownmixAndOverlapHelper::processStereoSamples(const CSAMPLE* pInput, size_t inputStereoSamples) {
    const size_t numInputFrames = inputStereoSamples / 2;
    return processInner(pInput, numInputFrames);
}

bool DownmixAndOverlapHelper::finalize() {
    // We need to append at least m_windowSize / 2 - m_stepSize silence
    // to have a valid analysis results for the last track samples.
    // Since we proceed in fixed steps, up to "m_stepSize - 1" sample remain
    // unprocessed. That is the reason why we use "m_windowSize / 2 - 1" below,
    // instead of "m_windowSize / 2 - m_stepSize"
    size_t framesToFillWindow = m_windowSize - m_bufferWritePosition;
    size_t numInputFrames = math_max(framesToFillWindow, m_windowSize / 2 - 1);
    return processInner(nullptr, numInputFrames);
}

bool DownmixAndOverlapHelper::processInner(
        const CSAMPLE* pInput, size_t numInputFrames) {
    size_t inRead = 0;
    double* pDownmix = m_buffer.data();

    while (inRead < numInputFrames) {
        size_t writeAvailable = math_min(numInputFrames,
                m_windowSize - m_bufferWritePosition);

        if (pInput) {
            for (size_t i = 0; i < writeAvailable; ++i) {
                // We analyze a mono downmix of the signal since we don't think
                // stereo does us any good.
                pDownmix[m_bufferWritePosition + i] = (pInput[(inRead + i) * 2] +
                                                              pInput[(inRead + i) * 2 + 1]) *
                        0.5;
            }
        } else {
            // we are in the finalize call. Add silence to
            // complete samples left in th buffer.
            for (size_t i = 0; i < writeAvailable; ++i) {
                pDownmix[m_bufferWritePosition + i] = 0;
            }
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

} // namespace mixxx
