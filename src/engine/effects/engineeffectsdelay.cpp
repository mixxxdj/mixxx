#include "engine/effects/engineeffectsdelay.h"

#include "util/rampingvalue.h"
#include "util/sample.h"

namespace {
// See enginedelay.cpp
const int kiMaxDelay = static_cast<int>(0.508 *
        mixxx::audio::SampleRate::kValueMax * mixxx::kEngineChannelCount);
} // anonymous namespace

EngineEffectsDelay::EngineEffectsDelay()
        : m_currentDelaySamples(0),
          m_prevDelaySamples(0),
          m_delayBufferWritePos(0) {
    m_pDelayBuffer = SampleUtil::alloc(kiMaxDelay);
}

void EngineEffectsDelay::process(CSAMPLE* pInOut,
        const int iBufferSize) {
    if (m_prevDelaySamples == 0 && m_currentDelaySamples == 0) {
        return;
    }

    int delaySourcePos =
            (m_delayBufferWritePos + kiMaxDelay - m_currentDelaySamples) %
            kiMaxDelay;

    VERIFY_OR_DEBUG_ASSERT(delaySourcePos >= 0) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(delaySourcePos <= static_cast<int>(kiMaxDelay)) {
        return;
    }

    if (m_prevDelaySamples == m_currentDelaySamples) {
        for (int i = 0; i < iBufferSize; ++i) {
            // Put samples into delay buffer.
            m_pDelayBuffer[m_delayBufferWritePos] = pInOut[i];
            m_delayBufferWritePos = (m_delayBufferWritePos + 1) % kiMaxDelay;

            // Take a delayed sample from the delay buffer
            // and copy it to the destination buffer.
            pInOut[i] = m_pDelayBuffer[delaySourcePos];
            delaySourcePos = (delaySourcePos + 1) % kiMaxDelay;
        }

    } else {
        VERIFY_OR_DEBUG_ASSERT(m_currentDelaySamples >= 0) {
            return;
        }

        int oldDelaySourcePos =
                (m_delayBufferWritePos + kiMaxDelay - m_prevDelaySamples) %
                kiMaxDelay;

        VERIFY_OR_DEBUG_ASSERT(oldDelaySourcePos >= 0) {
            return;
        }
        VERIFY_OR_DEBUG_ASSERT(oldDelaySourcePos <= static_cast<int>(kiMaxDelay)) {
            return;
        }

        RampingValue<CSAMPLE_GAIN> delayChangeRamped(0.0f, 1.0f, iBufferSize);

        for (int i = 0; i < iBufferSize; ++i) {
            // Put samples into delay buffer.
            m_pDelayBuffer[m_delayBufferWritePos] = pInOut[i];
            m_delayBufferWritePos = (m_delayBufferWritePos + 1) % kiMaxDelay;

            // Take delayed samples from the delay buffer
            // and with the use of ramping (cross-fading),
            // calculate the result sample value
            // and put it into the dest buffer.
            CSAMPLE_GAIN crossMix = delayChangeRamped.getNth(i);

            pInOut[i] = static_cast<CSAMPLE>(
                    m_pDelayBuffer[oldDelaySourcePos] * (1.0f - crossMix));
            pInOut[i] += static_cast<CSAMPLE>(m_pDelayBuffer[delaySourcePos] * crossMix);

            oldDelaySourcePos = (oldDelaySourcePos + 1) % kiMaxDelay;
            delaySourcePos = (delaySourcePos + 1) % kiMaxDelay;
        }

        m_prevDelaySamples = m_currentDelaySamples;
    }
}
