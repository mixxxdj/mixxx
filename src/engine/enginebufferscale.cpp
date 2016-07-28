#include "engine/enginebufferscale.h"

#include "util/defs.h"
#include "util/sample.h"

EngineBufferScale::EngineBufferScale()
        : m_audioSignal(
                Mixxx::AudioSignal::SampleLayout::Interleaved,
                Mixxx::AudioSignal::kChannelCountStereo,
                Mixxx::AudioSignal::kSamplingRateCD),
          m_dBaseRate(1.0),
          m_bSpeedAffectsPitch(false),
          m_dTempoRatio(1.0),
          m_dPitchRatio(1.0) {
    DEBUG_ASSERT(m_audioSignal.isValid());
}

EngineBufferScale::~EngineBufferScale() {
}

void EngineBufferScale::setSampleRate(SINT iSampleRate) {
    m_audioSignal = Mixxx::AudioSignal(
            m_audioSignal.getSampleLayout(),
            m_audioSignal.getChannelCount(),
            iSampleRate);
    DEBUG_ASSERT(m_audioSignal.isValid());
}
