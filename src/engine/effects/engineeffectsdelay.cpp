#include "engine/effects/engineeffectsdelay.h"

#include "moc_engineeffectsdelay.cpp"
#include "util/rampingvalue.h"
#include "util/sample.h"

EngineEffectsDelay::EngineEffectsDelay()
        : m_currentDelaySamples(0),
          m_prevDelaySamples(0),
          m_delayBufferWritePos(0) {
    m_pDelayBuffer = SampleUtil::alloc(kDelayBufferSize);
    SampleUtil::clear(m_pDelayBuffer, kDelayBufferSize);
}

EngineEffectsDelay::~EngineEffectsDelay() {
    SampleUtil::free(m_pDelayBuffer);
}

void EngineEffectsDelay::process(CSAMPLE* pInOut,
        const int iBufferSize) {
    if (m_prevDelaySamples == 0 && m_currentDelaySamples == 0) {
        for (int i = 0; i < iBufferSize; ++i) {
            // Put samples into delay buffer.
            m_pDelayBuffer[m_delayBufferWritePos] = pInOut[i];
            m_delayBufferWritePos = (m_delayBufferWritePos + 1) % kDelayBufferSize;
        }

        return;
    }

    // The "+ kDelayBufferSize" addition ensures positive values for the modulo calculation.
    // From a mathematical point of view, this addition can be removed. Anyway,
    // from the cpp point of view, the modulo operator for negative values
    // (for example, x % y, where x is a negative value) produces negative results
    // (but in math the result value is positive).
    int delaySourcePos =
            (m_delayBufferWritePos + kDelayBufferSize - m_currentDelaySamples) %
            kDelayBufferSize;

    if (m_prevDelaySamples == m_currentDelaySamples) {
        for (int i = 0; i < iBufferSize; ++i) {
            // Put samples into delay buffer.
            m_pDelayBuffer[m_delayBufferWritePos] = pInOut[i];
            m_delayBufferWritePos = (m_delayBufferWritePos + 1) % kDelayBufferSize;

            // Take a delayed sample from the delay buffer
            // and copy it to the destination buffer.
            pInOut[i] = m_pDelayBuffer[delaySourcePos];
            delaySourcePos = (delaySourcePos + 1) % kDelayBufferSize;
        }

    } else {
        // The "+ kDelayBufferSize" addition ensures positive values for the modulo calculation.
        // From a mathematical point of view, this addition can be removed. Anyway,
        // from the cpp point of view, the modulo operator for negative values
        // (for example, x % y, where x is a negative value) produces negative results
        // (but in math the result value is positive).
        int oldDelaySourcePos =
                (m_delayBufferWritePos + kDelayBufferSize - m_prevDelaySamples) %
                kDelayBufferSize;

        const RampingValue<CSAMPLE_GAIN> delayChangeRamped(0.0f, 1.0f, iBufferSize);

        for (int i = 0; i < iBufferSize; ++i) {
            // Put samples into delay buffer.
            m_pDelayBuffer[m_delayBufferWritePos] = pInOut[i];
            m_delayBufferWritePos = (m_delayBufferWritePos + 1) % kDelayBufferSize;

            // Take delayed samples from the delay buffer
            // and with the use of ramping (cross-fading),
            // calculate the result sample value
            // and put it into the dest buffer.
            CSAMPLE_GAIN crossMix = delayChangeRamped.getNth(i);

            pInOut[i] = m_pDelayBuffer[oldDelaySourcePos] * (1.0f - crossMix);
            pInOut[i] += m_pDelayBuffer[delaySourcePos] * crossMix;

            oldDelaySourcePos = (oldDelaySourcePos + 1) % kDelayBufferSize;
            delaySourcePos = (delaySourcePos + 1) % kDelayBufferSize;
        }

        m_prevDelaySamples = m_currentDelaySamples;
    }
}
