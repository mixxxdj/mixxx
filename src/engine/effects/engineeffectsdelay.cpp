#include "engine/effects/engineeffectsdelay.h"

#include <span>

#include "util/sample.h"
#include "util/span.h"

EngineEffectsDelay::EngineEffectsDelay()
        : m_currentDelaySamples(0),
          m_prevDelaySamples(0),
          m_currentDelayBuffer(kDelayBufferSize) {
    m_pDelayBuffer = std::make_unique<RingDelayBuffer>(kDelayBufferSize);
}

void EngineEffectsDelay::process(CSAMPLE* pInOut,
        const int iBufferSize) {
    std::span<CSAMPLE> inOutSpan = mixxx::spanutil::spanFromPtrLen(pInOut, iBufferSize);

    if (m_prevDelaySamples == 0 && m_currentDelaySamples == 0) {
        // TODO(davidchocholaty) check the returned number of written samples
        m_pDelayBuffer->write(inOutSpan);

        return;
    }

    if (m_prevDelaySamples == m_currentDelaySamples) {
        // TODO(davidchocholaty) check the returned number of written samples
        m_pDelayBuffer->write(inOutSpan);
        // TODO(davidchocholaty) check the returned number of read samples
        m_pDelayBuffer->read(inOutSpan, m_currentDelaySamples);
    } else {
        // TODO(davidchocholaty) check the returned number of written samples
        m_pDelayBuffer->write(inOutSpan);

        // TODO(davidchocholaty) check the returned number of read samples
        // Read the samples using the previous group delay samples.
        m_pDelayBuffer->read(inOutSpan, m_prevDelaySamples);
        // Read the samples using the current group delay samples.
        m_pDelayBuffer->read(
                m_currentDelayBuffer.span().subspan(0, iBufferSize),
                m_currentDelaySamples);

        SampleUtil::linearCrossfadeBuffersOut(
                pInOut,
                m_currentDelayBuffer.data(),
                iBufferSize);

        m_prevDelaySamples = m_currentDelaySamples;
    }
}
