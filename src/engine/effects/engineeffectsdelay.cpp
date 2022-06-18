#include "engine/effects/engineeffectsdelay.h"

#include "util/sample.h"

void EngineEffectsDelay::setDelay(unsigned int delaySamples) {
    m_delaySamples = delaySamples;
}

void EngineEffectsDelay::process(const CSAMPLE* pIn, CSAMPLE* pOutput, const int iBufferSize) {
    if (m_oldDelaySamples == 0 && m_delaySamples == 0) {
        if (pIn != pOutput) {
            SampleUtil::copy(pOutput, pIn, iBufferSize);
        }

        return;
    }

    if (m_oldDelaySamples == m_delaySamples &&
            m_delayBufferSize == iBufferSize + m_delaySamples) {
        int delaySourcePos = (m_delayPos + m_delayBufferSize - m_delaySamples) % m_delayBufferSize;

        VERIFY_OR_DEBUG_ASSERT(delaySourcePos >= 0) {
            if (pIn != pOutput) {
                SampleUtil::copy(pOutput, pIn, iBufferSize);
            }

            return;
        }
        VERIFY_OR_DEBUG_ASSERT(delaySourcePos <= static_cast<int>(m_delayBufferSize)) {
            if (pIn != pOutput) {
                SampleUtil::copy(pOutput, pIn, iBufferSize);
            }

            return;
        }

        for (int i = 0; i < iBufferSize; ++i) {
            // Put samples into delay buffer.
            m_delayBuffer[m_delayPos] = pIn[i];
            m_delayPos = (m_delayPos + 1) % m_delayBufferSize;

            // Take a delayed sample from the delay buffer and copy it to the destination buffer.
            pOutput[i] = m_delayBuffer[delaySourcePos];
            delaySourcePos = (delaySourcePos + 1) % m_delayBufferSize;
        }

    } else {
        // Size of the new delay buffer.
        int newDelayBufferSize = iBufferSize + m_delaySamples;

        if (m_delayBufferSize == 0) {
            // Creates first delay buffer and sets all elements to zero value.

            m_delayBuffer = new CSAMPLE[newDelayBufferSize];
            m_delayBufferSize = newDelayBufferSize;

            memset(m_delayBuffer, 0, m_delayBufferSize);
        } else {
            // Resize the delay buffer size. The elements from the delay buffer are copied
            // to the new one and the new delay buffer is set to the current delay buffer.

            CSAMPLE* newDelayBuffer = new CSAMPLE[newDelayBufferSize];

            // Actual delay position + samples that needs to be copied from the old delay buffer.
            // There are two possible situations:
            //
            // 1. The new delay buffer is greater than the old one
            //    - All elements from the delay buffer are copied to the new delay buffer.
            // 2. The new delay buffer is smaller than the old one
            //    - The all old delay buffer elements that are possible to store in the new buffer
            //      are copied.
            int endDelayPos = m_delayPos + math_min(m_delayBufferSize, newDelayBufferSize);

            while (m_delayPos < endDelayPos) {
                newDelayBuffer[m_delayPos % newDelayBufferSize] =
                        m_delayBuffer[m_delayPos % m_delayBufferSize];
                m_delayPos++;
            }

            // Recalculate the actual delay position for the new delay buffer.
            m_delayPos %= newDelayBufferSize;

            // Delete old buffer elements and replace this buffer with the new delay buffer.
            delete[] m_delayBuffer;
            m_delayBuffer = newDelayBuffer;

            m_delayBufferSize = newDelayBufferSize;
        }

        int delaySourcePos = (m_delayPos + m_delayBufferSize - m_delaySamples +
                                     iBufferSize / 2) %
                m_delayBufferSize;
        int oldDelaySourcePos =
                (m_delayPos + m_delayBufferSize - m_oldDelaySamples) %
                m_delayBufferSize;

        VERIFY_OR_DEBUG_ASSERT(delaySourcePos >= 0) {
            if (pIn != pOutput) {
                SampleUtil::copy(pOutput, pIn, iBufferSize);
            }

            return;
        }
        VERIFY_OR_DEBUG_ASSERT(delaySourcePos <= static_cast<int>(m_delayBufferSize)) {
            if (pIn != pOutput) {
                SampleUtil::copy(pOutput, pIn, iBufferSize);
            }

            return;
        }
        VERIFY_OR_DEBUG_ASSERT(oldDelaySourcePos >= 0) {
            if (pIn != pOutput) {
                SampleUtil::copy(pOutput, pIn, iBufferSize);
            }

            return;
        }
        VERIFY_OR_DEBUG_ASSERT(oldDelaySourcePos <= static_cast<int>(m_delayBufferSize)) {
            if (pIn != pOutput) {
                SampleUtil::copy(pOutput, pIn, iBufferSize);
            }

            return;
        }

        double crossMix = 0.0;
        double crossInc = 2 / static_cast<double>(iBufferSize);

        for (int i = 0; i < iBufferSize; ++i) {
            // Put samples into delay buffer.
            m_delayBuffer[m_delayPos] = pIn[i];
            m_delayPos = (m_delayPos + 1) % m_delayBufferSize;

            // Take delayed sample from delay buffer and copy it to dest buffer.
            if (i < iBufferSize / 2) {
                // Ramp only the second half of the buffer.
                pOutput[i] = m_delayBuffer[oldDelaySourcePos];
            } else {
                pOutput[i] = static_cast<CSAMPLE>(
                        m_delayBuffer[oldDelaySourcePos] * (1.0 - crossMix));
                pOutput[i] += static_cast<CSAMPLE>(m_delayBuffer[delaySourcePos] * crossMix);
                delaySourcePos = (delaySourcePos + 1) % m_delayBufferSize;

                crossMix += crossInc;
            }

            oldDelaySourcePos = (oldDelaySourcePos + 1) % m_delayBufferSize;
        }

        m_oldDelaySamples = m_delaySamples;
    }
}
