#ifndef ANALYZER_PLUGINS_BUFFERING_UTILS_H
#define ANALYZER_PLUGINS_BUFFERING_UTILS_H

#include "util/math.h"

template <typename ConvertTo, typename ConvertFrom>
class DownmixAndOverlapHelper {
    typedef std::function<bool(ConvertTo* pBuffer, size_t frames)> BlockReadyCallback;
  public:
    DownmixAndOverlapHelper() { }

    bool initialize(size_t blockSize, size_t stepSize, BlockReadyCallback callback) {
        m_buffer.resize(blockSize);
        m_callback = callback;
        m_blockSize = blockSize;
        m_stepSize = stepSize;
        m_bufferWritePosition = 0;
        return m_blockSize > 0 && m_stepSize > 0 &&
                m_stepSize <= m_blockSize && callback;
    }


    bool processStereoSamples(const ConvertFrom* pInput, size_t inputStereoSamples) {
        const size_t numInputFrames = inputStereoSamples / 2;
        size_t inRead = 0;
        ConvertTo* pDownmix = m_buffer.data();

        while (inRead < numInputFrames) {
            size_t writeAvailable = math_min(numInputFrames,
                                             m_blockSize - m_bufferWritePosition);

            for (size_t i = 0; i < writeAvailable; ++i) {
                // We analyze a mono downmix of the signal since we don't think
                // stereo does us any good.
                pDownmix[m_bufferWritePosition + i] = (pInput[(inRead + i) * 2] +
                                                       pInput[(inRead + i) * 2 + 1]) * 0.5;
            }
            m_bufferWritePosition += writeAvailable;
            inRead += writeAvailable;

            if (m_bufferWritePosition == m_blockSize) {
                bool result = m_callback(pDownmix, m_blockSize);

                // If the callback said not to continue then stop.
                if (!result) {
                    return false;
                }

                // If the block size equals the step size then we don't have to
                // do any copying.
                if (m_stepSize == m_blockSize) {
                    m_bufferWritePosition = 0;
                } else {
                    for (size_t i = 0; i < (m_blockSize - m_stepSize); ++i) {
                        pDownmix[i] = pDownmix[i + m_stepSize];
                    }
                    m_bufferWritePosition -= m_stepSize;
                }
            }
        }
        return true;
    }

    bool finalize() {
        // TODO(rryan) flush support?
        return true;
    }

  private:
    std::vector<ConvertTo> m_buffer;
    size_t m_blockSize = 0;
    size_t m_stepSize = 0;
    size_t m_bufferWritePosition = 0;
    BlockReadyCallback m_callback;
};

#endif /* ANALYZER_PLUGINS_BUFFERING_UTILS_H */
