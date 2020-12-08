#include "engine/bufferscalers/enginebufferscale.h"

#include "engine/engine.h"
#include "moc_enginebufferscale.cpp"
#include "util/defs.h"

EngineBufferScale::EngineBufferScale()
        : m_outputSignal(
                  mixxx::audio::SignalInfo(
                          mixxx::kEngineChannelCount,
                          mixxx::audio::SampleRate(),
                          mixxx::kEngineSampleLayout)),
          m_dBaseRate(1.0),
          m_bSpeedAffectsPitch(false),
          m_dTempoRatio(1.0),
          m_dPitchRatio(1.0) {
    DEBUG_ASSERT(!m_outputSignal.isValid());
}

void EngineBufferScale::setSampleRate(
        mixxx::audio::SampleRate sampleRate) {
    DEBUG_ASSERT(sampleRate.isValid());
    if (sampleRate != m_outputSignal.getSampleRate()) {
        m_outputSignal.setSampleRate(sampleRate);
        onSampleRateChanged();
    }
    DEBUG_ASSERT(m_outputSignal.isValid());
}
