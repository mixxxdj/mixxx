#pragma once

#include "util/audiosignal.h"


namespace mixxx {
    // TODO(XXX): When we move from stereo to multi-channel this needs updating.
    static constexpr mixxx::AudioSignal::ChannelCount kEngineChannelCount(2);

    // Contains the information needed to process a buffer of audio
    class AudioParameters : public AudioSignal {
      public:
        SINT framesPerBuffer() const {
            return m_framesPerBuffer;
        }
        SINT bufferSize() const {
            return static_cast<SINT>(channelCount()) * framesPerBuffer();
        }

        explicit AudioParameters(
                SampleLayout sampleLayout,
                ChannelCount channelCount,
                SampleRate sampleRate,
                SINT framesPerBuffer)
          : AudioSignal(sampleLayout, channelCount, sampleRate),
            m_framesPerBuffer(framesPerBuffer) {
            DEBUG_ASSERT(framesPerBuffer > 0);
        }

      private:
        SINT m_framesPerBuffer;
    };
}
