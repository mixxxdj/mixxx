#pragma once

#include "audio/signalinfo.h"


namespace mixxx {
static constexpr audio::ChannelCount kEngineChannelOutputCount =
        audio::ChannelCount::stereo();
static constexpr audio::ChannelCount kMaxEngineChannelInputCount =
        audio::ChannelCount::stem();

// Contains the information needed to process a buffer of audio
class EngineParameters final {
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
                      kEngineChannelOutputCount,
                      sampleRate),
              m_framesPerBuffer(framesPerBuffer) {
        DEBUG_ASSERT(framesPerBuffer > 0);
        DEBUG_ASSERT(sampleRate > 0);
    }

  private:
    const audio::SignalInfo m_outputSignal;
    const SINT m_framesPerBuffer;
};
}
