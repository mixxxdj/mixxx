#include "engine/effects/engineeffectsdelay.h"

#include "util/sample.h"

namespace {
// See enginedelay.cpp
const int kiMaxDelay = static_cast<int>(0.508 *
        mixxx::audio::SampleRate::kValueMax * mixxx::kEngineChannelCount);
} // anonymous namespace

EngineEffectsDelay::EngineEffectsDelay()
        : m_delaySamples(0),
          m_oldDelaySamples(0),
          m_delayPos(0) {
    m_pDelayBuffer = SampleUtil::alloc(kiMaxDelay);
}

void EngineEffectsDelay::process(const CSAMPLE* M_RESTRICT pIn,
        CSAMPLE* M_RESTRICT pOutput,
        const int iBufferSize) {
    if (m_oldDelaySamples == 0 && m_delaySamples == 0) {
        SampleUtil::copy(pOutput, pIn, iBufferSize);
        return;
    }

    if (m_oldDelaySamples == m_delaySamples) {
        int delaySourcePos = (m_delayPos + kiMaxDelay - m_delaySamples) % kiMaxDelay;

        VERIFY_OR_DEBUG_ASSERT(delaySourcePos >= 0) {
            SampleUtil::copy(pOutput, pIn, iBufferSize);
            return;
        }
        VERIFY_OR_DEBUG_ASSERT(delaySourcePos <= static_cast<int>(kiMaxDelay)) {
            SampleUtil::copy(pOutput, pIn, iBufferSize);
            return;
        }

        for (int i = 0; i < iBufferSize; ++i) {
            // Put samples into delay buffer.
            m_pDelayBuffer[m_delayPos] = pIn[i];
            m_delayPos = (m_delayPos + 1) % kiMaxDelay;

            // Take a delayed sample from the delay buffer
            // and copy it to the destination buffer.
            pOutput[i] = m_pDelayBuffer[delaySourcePos];
            delaySourcePos = (delaySourcePos + 1) % kiMaxDelay;
        }

    } else {
        int delaySourcePos = (m_delayPos + kiMaxDelay - m_delaySamples +
                                     iBufferSize / 2) %
                kiMaxDelay;
        int oldDelaySourcePos =
                (m_delayPos + kiMaxDelay - m_oldDelaySamples) %
                kiMaxDelay;

        VERIFY_OR_DEBUG_ASSERT(delaySourcePos >= 0) {
            SampleUtil::copy(pOutput, pIn, iBufferSize);
            return;
        }
        VERIFY_OR_DEBUG_ASSERT(delaySourcePos <= static_cast<int>(kiMaxDelay)) {
            SampleUtil::copy(pOutput, pIn, iBufferSize);
            return;
        }
        VERIFY_OR_DEBUG_ASSERT(oldDelaySourcePos >= 0) {
            SampleUtil::copy(pOutput, pIn, iBufferSize);
            return;
        }
        VERIFY_OR_DEBUG_ASSERT(oldDelaySourcePos <= static_cast<int>(kiMaxDelay)) {
            SampleUtil::copy(pOutput, pIn, iBufferSize);
            return;
        }

        double crossMix = 0.0;
        double crossInc = 2 / static_cast<double>(iBufferSize);

        for (int i = 0; i < iBufferSize; ++i) {
            // Put samples into delay buffer.
            m_pDelayBuffer[m_delayPos] = pIn[i];
            m_delayPos = (m_delayPos + 1) % kiMaxDelay;

            // Take delayed sample from delay buffer and copy it to dest buffer.
            if (i < iBufferSize / 2) {
                // Ramp only the second half of the buffer.
                pOutput[i] = m_pDelayBuffer[oldDelaySourcePos];
            } else {
                pOutput[i] = static_cast<CSAMPLE>(
                        m_pDelayBuffer[oldDelaySourcePos] * (1.0 - crossMix));
                pOutput[i] += static_cast<CSAMPLE>(m_pDelayBuffer[delaySourcePos] * crossMix);
                delaySourcePos = (delaySourcePos + 1) % kiMaxDelay;

                crossMix += crossInc;
            }

            oldDelaySourcePos = (oldDelaySourcePos + 1) % kiMaxDelay;
        }

        m_oldDelaySamples = m_delaySamples;
    }
}
