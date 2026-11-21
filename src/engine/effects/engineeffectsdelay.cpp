#include "engine/effects/engineeffectsdelay.h"

#include <algorithm>
#include <cstring>

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
        const std::size_t bufferSize) {
    if (m_prevDelaySamples == 0 && m_currentDelaySamples == 0) {
        // Put samples into the delay buffer: at most two block copies (the end
        // of the ring buffer splits the block). This is what every effect
        // chain does at each callback while no effect is delayed; per sample
        // with a modulo it was 9 % of the audio thread on ARM64 with two stem
        // decks playing.
        const CSAMPLE* pIn = pInOut;
        std::size_t remaining = bufferSize;
        while (remaining > 0) {
            const std::size_t toEnd = static_cast<std::size_t>(
                    kDelayBufferSize - m_delayBufferWritePos);
            const std::size_t chunk = std::min(remaining, toEnd);
            std::memcpy(m_pDelayBuffer + m_delayBufferWritePos, pIn, chunk * sizeof(CSAMPLE));
            m_delayBufferWritePos = static_cast<SINT>(
                    (static_cast<std::size_t>(m_delayBufferWritePos) + chunk) %
                    static_cast<std::size_t>(kDelayBufferSize));
            pIn += chunk;
            remaining -= chunk;
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
        for (std::size_t i = 0; i < bufferSize; ++i) {
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

        const RampingValue<CSAMPLE_GAIN> delayChangeRamped(
                0.0f, 1.0f, static_cast<int>(bufferSize));

        for (std::size_t i = 0; i < bufferSize; ++i) {
            // Put samples into delay buffer.
            m_pDelayBuffer[m_delayBufferWritePos] = pInOut[i];
            m_delayBufferWritePos = (m_delayBufferWritePos + 1) % kDelayBufferSize;

            // Take delayed samples from the delay buffer
            // and with the use of ramping (cross-fading),
            // calculate the result sample value
            // and put it into the dest buffer.
            CSAMPLE_GAIN crossMix = delayChangeRamped.getNth(static_cast<int>(i));

            pInOut[i] = m_pDelayBuffer[oldDelaySourcePos] * (1.0f - crossMix);
            pInOut[i] += m_pDelayBuffer[delaySourcePos] * crossMix;

            oldDelaySourcePos = (oldDelaySourcePos + 1) % kDelayBufferSize;
            delaySourcePos = (delaySourcePos + 1) % kDelayBufferSize;
        }

        m_prevDelaySamples = m_currentDelaySamples;
    }
}
