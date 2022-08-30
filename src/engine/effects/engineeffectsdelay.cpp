#include "engine/effects/engineeffectsdelay.h"

#include <span>

#include "util/sample.h"
#include "util/span.h"

EngineEffectsDelay::EngineEffectsDelay()
        : m_currentDelaySamples(0),
          m_prevDelaySamples(0),
          m_currentDelayBuffer(kDelayBufferSize),
          m_delayBuffer(kDelayBufferSize) {
}

void EngineEffectsDelay::process(CSAMPLE* pInOut,
        const int iBufferSize) {
    std::span<CSAMPLE> inOutSpan = mixxx::spanutil::spanFromPtrLen(pInOut, iBufferSize);

    if (m_prevDelaySamples == 0 && m_currentDelaySamples == 0) {
        // TODO(davidchocholaty) check the returned number of written samples
        m_delayBuffer.write(inOutSpan);

        return;
    }

    if (m_prevDelaySamples == m_currentDelaySamples) {
        // TODO(davidchocholaty) check the returned number of written samples
        m_delayBuffer.write(inOutSpan);
        // TODO(davidchocholaty) check the returned number of read samples
        m_delayBuffer.read(inOutSpan, m_currentDelaySamples);
    } else {
        // TODO(davidchocholaty) check the returned number of written samples
        m_delayBuffer.write(inOutSpan);

        // TODO(davidchocholaty) check the returned number of read samples
        // Read the samples using the previous group delay samples.
        m_delayBuffer.read(inOutSpan, m_prevDelaySamples);

        // Read the samples using the current group delay samples.
        auto tmpBufferView = m_currentDelayBuffer.span().first(inOutSpan.size());
        m_delayBuffer.read(tmpBufferView, m_currentDelaySamples);

        SampleUtil::linearCrossfadeBuffersOut(
                pInOut,
                tmpBufferView.data(),
                tmpBufferView.size());

        m_prevDelaySamples = m_currentDelaySamples;
    }
}
