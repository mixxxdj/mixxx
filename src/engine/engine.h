#pragma once

#include "util/audiosignal.h"


namespace mixxx {
    // TODO(XXX): When we move from stereo to multi-channel this needs updating.
    static constexpr mixxx::AudioSignal::ChannelCount kEngineChannelCount(2);

    // Contains the information needed to process a buffer of audio
    class EngineParameters {
      public:
        SINT framesPerBuffer() const {
            return m_framesPerBuffer;
        }
        SINT samplesPerBuffer() const {
            return m_audioSignal.frames2samples(framesPerBuffer());
        }

        mixxx::AudioSignal::ChannelCount channelCount() const {
            return m_audioSignal.channelCount();
        }

        mixxx::AudioSignal::SampleRate sampleRate() const {
            return m_audioSignal.sampleRate();
        }

        explicit EngineParameters(
                AudioSignal::SampleRate sampleRate,
                SINT framesPerBuffer)
          : m_audioSignal(mixxx::AudioSignal::SampleLayout::Interleaved,
                          kEngineChannelCount, sampleRate),
            m_framesPerBuffer(framesPerBuffer) {
            DEBUG_ASSERT(framesPerBuffer > 0);
        }

      private:
        const mixxx::AudioSignal m_audioSignal;
        const SINT m_framesPerBuffer;
    };
}
