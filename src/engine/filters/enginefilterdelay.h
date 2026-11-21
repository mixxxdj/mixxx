#pragma once

#include <algorithm>
#include <cstddef>
#include <cstring>

#include "engine/engine.h"
#include "engine/engineobject.h"
#include "util/assert.h"
#include "util/sample.h"

template<unsigned int SIZE>
class EngineFilterDelay : public EngineObjectConstIn {
    static_assert(SIZE % mixxx::kEngineChannelOutputCount == 0,
            "The buffer size has to be divisible by the number of channels.");

  public:
    EngineFilterDelay()
            : m_delaySamples(0),
              m_oldDelaySamples(0),
              m_delayPos(0),
              m_doStart(false) {
        // Set the current buffers to 0
        memset(m_buf, 0, sizeof(m_buf));
    }

    virtual ~EngineFilterDelay() {};

    void pauseFilter() {
        // Set the current buffers to 0
        if (!m_doStart) {
            memset(m_buf, 0, sizeof(m_buf));
            m_oldDelaySamples = 0;
            m_doStart = true;
        }
    }

    void setDelay(unsigned int delaySamples) {
        unsigned int unalignedSamples = delaySamples % mixxx::kEngineChannelOutputCount;

        VERIFY_OR_DEBUG_ASSERT(unalignedSamples == 0) {
            // Round to the previous multiple of the number of channel count.
            delaySamples = delaySamples - unalignedSamples;
        }

        VERIFY_OR_DEBUG_ASSERT(delaySamples < SIZE) {
            delaySamples = SIZE - mixxx::kEngineChannelOutputCount;
        }

        m_delaySamples = delaySamples;
    }

    virtual void process(const CSAMPLE* pIn, CSAMPLE* pOutput, const std::size_t bufferSize) {
        if (m_oldDelaySamples == m_delaySamples &&
                bufferSize + static_cast<std::size_t>(m_delaySamples) <= SIZE) {
            // Same result as the sample by sample loop below (every slot that is
            // read has already been written, because the whole buffer plus the
            // delay fits in the ring) but in block copies: that loop does two
            // modulo operations per sample, which made it 3 % of the audio
            // thread on an ARM64 device with two stem decks playing.
            const std::size_t srcStart = (static_cast<std::size_t>(m_delayPos) + SIZE -
                                                 static_cast<std::size_t>(m_delaySamples)) %
                    SIZE;
            std::size_t pos = static_cast<std::size_t>(m_delayPos);
            std::size_t remaining = bufferSize;
            const CSAMPLE* pFrom = pIn;
            while (remaining > 0) {
                const std::size_t chunk = std::min(remaining, SIZE - pos);
                std::memcpy(m_buf + pos, pFrom, chunk * sizeof(CSAMPLE));
                pos = (pos + chunk) % SIZE;
                pFrom += chunk;
                remaining -= chunk;
            }
            m_delayPos = static_cast<int>(pos);
            pos = srcStart;
            remaining = bufferSize;
            CSAMPLE* pTo = pOutput;
            while (remaining > 0) {
                const std::size_t chunk = std::min(remaining, SIZE - pos);
                std::memcpy(pTo, m_buf + pos, chunk * sizeof(CSAMPLE));
                pos = (pos + chunk) % SIZE;
                pTo += chunk;
                remaining -= chunk;
            }
        } else if (m_oldDelaySamples == m_delaySamples) {
            // The "+ SIZE" addition ensures positive values for the modulo calculation.
            // From a mathematical point of view, this addition can be removed. Anyway,
            // from the cpp point of view, the modulo operator for negative values
            // (for example, x % y, where x is a negative value) produces negative results
            // (but in math the result value is positive).
            int delaySourcePos = (m_delayPos + SIZE - m_delaySamples) % SIZE;

            for (std::size_t i = 0; i < bufferSize; ++i) {
                // put sample into delay buffer:
                m_buf[m_delayPos] = pIn[i];
                m_delayPos = (m_delayPos + 1) % SIZE;

                // Take delayed sample from delay buffer and copy it to dest buffer:
                pOutput[i] = m_buf[delaySourcePos];
                delaySourcePos = (delaySourcePos + 1) % SIZE;
            }
        } else {
            // The "+ SIZE" addition ensures positive values for the modulo calculation.
            // From a mathematical point of view, this addition can be removed. Anyway,
            // from the cpp point of view, the modulo operator for negative values
            // (for example, x % y, where x is a negative value) produces negative results
            // (but in math the result value is positive).
            int delaySourcePos = (m_delayPos + SIZE - m_delaySamples + bufferSize / 2) % SIZE;
            int oldDelaySourcePos = (m_delayPos + SIZE - m_oldDelaySamples) % SIZE;

            double cross_mix = 0.0;
            double cross_inc = 2 / static_cast<double>(bufferSize);

            for (std::size_t i = 0; i < bufferSize; ++i) {
                // put sample into delay buffer:
                m_buf[m_delayPos] = pIn[i];
                m_delayPos = (m_delayPos + 1) % SIZE;

                // Take delayed sample from delay buffer and copy it to dest buffer:
                if (i < bufferSize / 2) {
                    // only ramp the second half of the buffer, because we do
                    // the same in the IIR filter to wait for settling
                    pOutput[i] = m_buf[oldDelaySourcePos];
                } else {
                    pOutput[i] = static_cast<CSAMPLE>(m_buf[oldDelaySourcePos] * (1.0 - cross_mix));
                    pOutput[i] += static_cast<CSAMPLE>(m_buf[delaySourcePos] * cross_mix);
                    delaySourcePos = (delaySourcePos + 1) % SIZE;
                    cross_mix += cross_inc;
                }
                oldDelaySourcePos = (oldDelaySourcePos + 1) % SIZE;
            }
            m_oldDelaySamples = m_delaySamples;
        }
        m_doStart = false;
    }

    // this is can be used instead off a final process() call before pause
    // It fades to dry or 0 according to the m_startFromDry parameter
    // it is an alternative for using pauseFillter() calls
    void processAndPauseFilter(const CSAMPLE* pIn, CSAMPLE* pOutput, const std::size_t bufferSize) {
        int oldDelay = m_delaySamples;
        m_delaySamples = 0;
        process(pIn, pOutput, bufferSize);
        m_delaySamples = oldDelay;
        pauseFilter();
    }

  protected:
    int m_delaySamples;
    int m_oldDelaySamples;
    int m_delayPos;
    CSAMPLE m_buf[SIZE];
    bool m_doStart;
};
