#include "engine/effects/engineeffectsdelay.h"

#include "util/rampingvalue.h"
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
        VERIFY_OR_DEBUG_ASSERT(m_delaySamples >= 0) {
            SampleUtil::copy(pOutput, pIn, iBufferSize);
            return;
        }

        int delaySourcePos =
                (m_delayPos + kiMaxDelay - m_delaySamples) %
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

        RampingValue<CSAMPLE_GAIN> delayChangeRamped(0.0f, 1.0f, iBufferSize);

        for (int i = 0; i < iBufferSize; ++i) {
            // Put samples into delay buffer.
            m_pDelayBuffer[m_delayPos] = pIn[i];
            m_delayPos = (m_delayPos + 1) % kiMaxDelay;

            // Take delayed samples from the delay buffer
            // and with the use of ramping (cross-fading),
            // calculate the result sample value
            // and put it into the dest buffer.
            CSAMPLE_GAIN crossMix = delayChangeRamped.getNth(i);

            pOutput[i] = static_cast<CSAMPLE>(
                    m_pDelayBuffer[oldDelaySourcePos] * (1.0f - crossMix));
            pOutput[i] += static_cast<CSAMPLE>(m_pDelayBuffer[delaySourcePos] * crossMix);

            oldDelaySourcePos = (oldDelaySourcePos + 1) % kiMaxDelay;
            delaySourcePos = (delaySourcePos + 1) % kiMaxDelay;
        }

        m_oldDelaySamples = m_delaySamples;
    }
}
