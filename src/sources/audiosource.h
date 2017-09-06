#ifndef MIXXX_AUDIOSOURCE_H
#define MIXXX_AUDIOSOURCE_H

#include "sources/sampleframesource.h"
#include "sources/urlresource.h"
#include "util/memory.h"


namespace mixxx {

// Common interface and base class for audio sources.
//
// Both the number of channels and the sampling rate must
// be constant and are not allowed to change over time.
//
// Samples in a sample buffer are stored as consecutive frames,
// i.e. the samples of the channels are interleaved.
//
// Audio sources are implicitly opened upon creation and
// closed upon destruction.
class AudioSource: public UrlResource, public SampleFrameSource {
  public:
    // The bitrate is optional and measured in kbit/s (kbps).
    // It depends on the metadata and decoder if a value for the
    // bitrate is available.
    class Bitrate {
      private:
        static constexpr SINT kValueDefault = 0;

      public:
        static constexpr const char* unit() { return "kbps"; }

        explicit constexpr Bitrate(SINT value = kValueDefault)
            : m_value(value) {
        }

        bool valid() const {
            return m_value > kValueDefault;
        }

        /*implicit*/ operator SINT() const {
            DEBUG_ASSERT(m_value >= kValueDefault); // unsigned value
            return m_value;
        }

      private:
        SINT m_value;
    };

    Bitrate bitrate() const {
        return m_bitrate;
    }

    bool verifyReadable() const override;

  protected:
    explicit AudioSource(const QUrl& url);
    AudioSource(const AudioSource& other) = default;

    bool initBitrateOnce(Bitrate bitrate);
    bool initBitrateOnce(SINT bitrate) {
        return initBitrateOnce(Bitrate(bitrate));
    }

  private:
    Bitrate m_bitrate;
};

// Parameters for configuring audio sources
class AudioSourceConfig : public AudioSignal {
  public:
    AudioSourceConfig()
        : AudioSignal(SampleFrameSource::kSampleLayout) {
    }
    AudioSourceConfig(ChannelCount channelCount, SamplingRate samplingRate)
        : AudioSignal(SampleFrameSource::kSampleLayout, channelCount, samplingRate) {
    }

    using AudioSignal::setChannelCount;
    using AudioSignal::setSamplingRate;
};

typedef std::shared_ptr<AudioSource> AudioSourcePointer;

} // namespace mixxx


#endif // MIXXX_AUDIOSOURCE_H
