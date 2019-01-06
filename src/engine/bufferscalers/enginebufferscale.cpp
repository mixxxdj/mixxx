#include "engine/bufferscalers/enginebufferscale.h"

#include "engine/engine.h"
#include "util/defs.h"
#include "util/sample.h"

EngineBufferScale::EngineBufferScale()
        : m_audioSignal(
                mixxx::AudioSignal::SampleLayout::Interleaved,
                mixxx::AudioSignal::ChannelCount(mixxx::kEngineChannelCount),
                mixxx::AudioSignal::SampleRate(44100)),
          m_dBaseRate(1.0),
          m_bSpeedAffectsPitch(false),
          m_dTempoRatio(1.0),
          m_dPitchRatio(1.0) {
    DEBUG_ASSERT(m_audioSignal.verifyReadable());
}

EngineBufferScale::~EngineBufferScale() {
}

void EngineBufferScale::setSampleRate(SINT iSampleRate) {
    m_audioSignal = mixxx::AudioSignal(
            m_audioSignal.sampleLayout(),
            m_audioSignal.channelCount(),
            mixxx::AudioSignal::SampleRate(iSampleRate));
    DEBUG_ASSERT(m_audioSignal.verifyReadable());
}
