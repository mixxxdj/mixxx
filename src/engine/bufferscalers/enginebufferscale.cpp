#include "engine/bufferscalers/enginebufferscale.h"

#include "engine/engine.h"
#include "moc_enginebufferscale.cpp"
#include "soundio/soundmanagerconfig.h"

EngineBufferScale::EngineBufferScale()
        : m_outputSignal(
                  mixxx::audio::SignalInfo(
                          mixxx::kEngineChannelOutputCount,
                          mixxx::audio::SampleRate())),
          m_dBaseRate(1.0),
          m_bSpeedAffectsPitch(false),
          m_dTempoRatio(1.0),
          m_dPitchRatio(1.0),
          m_effectiveRate(1.0) {
    DEBUG_ASSERT(!m_outputSignal.isValid());
}

void EngineBufferScale::setOutputSignal(
        mixxx::audio::SampleRate sampleRate,
        mixxx::audio::ChannelCount channelCount) {
    DEBUG_ASSERT(sampleRate.isValid());
    DEBUG_ASSERT(channelCount.isValid());
    bool changed = false;
    if (sampleRate != m_outputSignal.getSampleRate()) {
        m_outputSignal.setSampleRate(sampleRate);
        changed = true;
    }
    if (channelCount != m_outputSignal.getChannelCount()) {
        m_outputSignal.setChannelCount(channelCount);
        changed = true;
    }
    if (changed) {
        onOutputSignalChanged();
    }
    DEBUG_ASSERT(m_outputSignal.isValid());
}
