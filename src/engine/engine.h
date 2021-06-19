#pragma once

#include "audio/signalinfo.h"


namespace mixxx {
    // TODO(XXX): When we move from stereo to multi-channel this needs updating.
static constexpr audio::ChannelCount kEngineChannelCount =
        audio::ChannelCount::stereo();

// Contains the information needed to process a buffer of audio
class EngineParameters {
  public:
    SINT framesPerBuffer() const {
        return m_framesPerBuffer;
    }
    SINT samplesPerBuffer() const {
        return m_outputSignal.frames2samples(framesPerBuffer());
    }

    audio::ChannelCount channelCount() const {
        return m_outputSignal.getChannelCount();
    }

    audio::SampleRate sampleRate() const {
        return m_outputSignal.getSampleRate();
    }

    explicit EngineParameters(
            audio::SampleRate sampleRate,
            SINT framesPerBuffer)
            : m_outputSignal(
                      kEngineChannelCount,
                      sampleRate),
              m_framesPerBuffer(framesPerBuffer) {
        DEBUG_ASSERT(framesPerBuffer > 0);
    }

  private:
    const audio::SignalInfo m_outputSignal;
    const SINT m_framesPerBuffer;
};
}
