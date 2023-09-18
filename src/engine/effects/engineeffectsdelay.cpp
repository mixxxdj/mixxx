#include "engine/effects/engineeffectsdelay.h"

#include <span>

#include "util/math.h"
#include "util/sample.h"
#include "util/span.h"

namespace {
SINT readWithMaxDelay(std::span<CSAMPLE> inOutSpan,
        RingDelayBuffer& delayBuffer) {
    // In this situation, the sum of the size of the inOutSpan
    // and the provided delay is greater than the ring buffer.
    // The wrong delay is minimized by using the maximum possible delay
    // for handling with the ring buffer.
    const SINT maxDelaySamples = math_max(
            kDelayBufferSize - static_cast<int>(inOutSpan.size()), 0);
    delayBuffer.read(inOutSpan, maxDelaySamples);

    return maxDelaySamples;
}

} // anonymous namespace

EngineEffectsDelay::EngineEffectsDelay()
        : m_currentDelaySamples(0),
          m_prevDelaySamples(0),
          m_currentDelayBuffer(kDelayBufferSize),
          m_delayBuffer(kDelayBufferSize) {
}

EngineEffectsDelay::~EngineEffectsDelay() {
    SampleUtil::free(m_pDelayBuffer);
}

void EngineEffectsDelay::process(CSAMPLE* pInOut,
        const int iBufferSize) {
    std::span<CSAMPLE> inOutSpan = mixxx::spanutil::spanFromPtrLen(pInOut, iBufferSize);

    if (m_prevDelaySamples == 0 && m_currentDelaySamples == 0) {
        // We can keep writing without reading due to the implementation
        // of the RingDelayBuffer. The implementation is specific,
        // that the RingDelayBuffer works only with the write position only
        // (not with the reading position). Based on that, we can keep writing
        // and the old unread data are rewritten with the new one.
        // So, the ring buffer can't be full.
        const SINT writtenSamples = m_delayBuffer.write(inOutSpan);
        VERIFY_OR_DEBUG_ASSERT(writtenSamples == iBufferSize) {
            // It is not needed to return from here
            // based on the return that follows after.
        }

        return;
    }

    if (m_prevDelaySamples == m_currentDelaySamples) {
        const SINT writtenSamples = m_delayBuffer.write(inOutSpan);
        VERIFY_OR_DEBUG_ASSERT(writtenSamples == iBufferSize) {
            return;
        }

        const SINT readSamples = m_delayBuffer.read(inOutSpan, m_currentDelaySamples);
        VERIFY_OR_DEBUG_ASSERT(readSamples == iBufferSize) {
            m_currentDelaySamples = readWithMaxDelay(inOutSpan, m_delayBuffer);
        }

    } else {
        const SINT writtenSamples = m_delayBuffer.write(inOutSpan);
        VERIFY_OR_DEBUG_ASSERT(writtenSamples == iBufferSize) {
            return;
        }

        // Read the samples using the previous group delay samples.
        SINT readSamples = m_delayBuffer.read(inOutSpan, m_prevDelaySamples);
        VERIFY_OR_DEBUG_ASSERT(readSamples == iBufferSize) {
            // In this situation, it is not possible that the delay
            // will be greater than possible. For the first call,
            // the m_prevDelaySamples value is 0 and before assigning
            // the m_currentDelaySamples into m_prevDelaySamples all delays
            // which are too big are clamped.

            // This VERIFY_OR_DEBUG_ASSERT can be false only for the situation,
            // where the inOutSpan size is just too much big
            // for the RingDelayBuffer size based on the incorrectly
            // chosen size.

            // Based on that, the inOutSpan is unchanged and the input data
            // are kept as output.
            return;
        }

        // Read the samples using the current group delay samples.
        auto tmpBufferView = m_currentDelayBuffer.span().first(inOutSpan.size());

        readSamples = m_delayBuffer.read(tmpBufferView, m_currentDelaySamples);
        VERIFY_OR_DEBUG_ASSERT(readSamples == iBufferSize) {
            m_currentDelaySamples = readWithMaxDelay(inOutSpan, m_delayBuffer);
        }

        SampleUtil::linearCrossfadeBuffersOut(
                pInOut,
                tmpBufferView.data(),
                tmpBufferView.size());

        m_prevDelaySamples = m_currentDelaySamples;
    }
}
