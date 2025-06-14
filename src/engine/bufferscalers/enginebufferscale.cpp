#include "engine/bufferscalers/enginebufferscale.h"

#include "engine/engine.h"
#include "moc_enginebufferscale.cpp"
#include "soundio/soundmanagerconfig.h"

EngineBufferScale::EngineBufferScale()
        : m_signal(
                  mixxx::audio::SignalInfo(
                          mixxx::kEngineChannelOutputCount,
                          mixxx::audio::SampleRate())),
          m_dBaseRate(10),
          m_bSpeedAffectsPitch(false),
          m_dTempoRatio(1.0),
          m_dPitchRatio(1.0),
          m_effectiveRate(1.0) {
    DEBUG_ASSERT(!m_signal.isValid());
}

void EngineBufferScale::setSignal(
        mixxx::audio::SampleRate sampleRate,
        mixxx::audio::ChannelCount channelCount) {
    DEBUG_ASSERT(sampleRate.isValid());
    DEBUG_ASSERT(channelCount.isValid());
    bool changed = false;
    if (sampleRate != m_signal.getSampleRate()) {
        m_signal.setSampleRate(sampleRate);
        changed = true;
    }
    if (channelCount != m_signal.getChannelCount()) {
        m_signal.setChannelCount(channelCount);
        changed = true;
    }
    if (changed) {
        onSignalChanged();
    }
    DEBUG_ASSERT(m_signal.isValid());
}
