#ifndef MIXXX_AUDIOSOURCE_H
#define MIXXX_AUDIOSOURCE_H

#include "sources/urlresource.h"
#include "util/audiosignal.h"
#include "util/indexrange.h"
#include "util/memory.h"
#include "util/result.h"
#include "util/samplebuffer.h"

namespace mixxx {

// forward declaration(s)
class AudioSource;
class AudioSourceConfig;

typedef std::shared_ptr<AudioSource> AudioSourcePointer;

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
class AudioSource: public UrlResource, public AudioSignal {
  public:
    // All AudioSources are required to produce a signal of frames
    // where each frame contains samples from all channels that are
    // coincident in time.
    //
    // A frame for a mono signal contains a single sample. A frame
    // for a stereo signal contains a pair of samples, one for the
    // left and right channel respectively.
    static constexpr SampleLayout sampleLayout() {
        return SampleLayout::Interleaved;
    }


    // The total length of audio data is bounded and measured in frames.
    IndexRange frameIndexRange() const {
        return m_frameIndexRange;
    }

    // The index of the first frame.
    SINT frameIndexMin() const {
        DEBUG_ASSERT(m_frameIndexRange.head() <= m_frameIndexRange.tail());
        return m_frameIndexRange.head();
    }

    // The index after the last frame.
    SINT frameIndexMax() const {
        DEBUG_ASSERT(m_frameIndexRange.head() <= m_frameIndexRange.tail());
        return m_frameIndexRange.tail();
    }

    // The sample frame index is valid within the range
    // [frameIndexMin(), frameIndexMax()]
    // including the upper bound of the range!
    bool isValidFrameIndex(SINT frameIndex) const {
        return m_frameIndexRange.clamp(frameIndex) == frameIndex;
    }


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


    // The actual duration in seconds.
    // Well defined only for valid files!
    inline bool hasDuration() const {
        return samplingRate().valid();
    }
    inline double getDuration() const {
        DEBUG_ASSERT(hasDuration()); // prevents division by zero
        return double(frameIndexRange().length()) / double(samplingRate());
    }


    bool verifyReadable() const override;


    ///////////////////////////////////////////////////////////////////////////
    // AudioSource v2 Interface
    ///////////////////////////////////////////////////////////////////////////

    // Read sample frames from the requested frame index range. Only
    // forward-oriented index ranges need to be supported. The implementation
    // is responsible for restricting the requested range to the range that
    // is actually readable.
    //
    // Only that part of the output buffer corresponding to the returned
    // range is allowed to be modified. All remaining samples in the output
    // buffer should stay untouched. The samples in the output buffer need
    // to be offset properly depending on the difference between the first
    // requested frame and the actual first read frame, i.e. the first sample
    // in the output buffer represents the sample of the first channel from
    // the first requested frame.
    //
    // If the output buffer is null all decoded samples must be discarded
    // immediately. Otherwise the size of the output buffer must be
    // sufficient to buffer all frames2samples(frameIndexRange.length())
    // sample values.
    //
    // On errors only a sub range of the requested frames might be returned.
    //
    // TODO(XXX): Declare as a pure virtual function after deleting the
    // default implementation that is currently used to support migration
    // to the new v2 API.
    virtual IndexRange readOrSkipSampleFrames(
            IndexRange frameIndexRange,
            SampleBuffer::WritableSlice* pOutputBuffer) /*= 0*/;

    /*non-virtual*/ IndexRange readSampleFrames(
            IndexRange frameIndexRange,
            SampleBuffer::WritableSlice outputBuffer) {
        return readOrSkipSampleFrames(frameIndexRange, &outputBuffer);
    }

    // Might be overridden by derived classes to provide an optimized
    // implementation for this special case.
    virtual IndexRange skipSampleFrames(
            IndexRange frameIndexRange) {
        return readOrSkipSampleFrames(frameIndexRange, nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////


  protected:
    ///////////////////////////////////////////////////////////////////////////
    // Legacy AudioSource Interface
    //
    // These functions are no longer used and should not be implemented for
    // new AudioSources!!
    //
    // TODO(XXX): Delete after all derived classes have been migrated
    // to the new v2 interface!
    ///////////////////////////////////////////////////////////////////////////

    // Adjusts the current frame seek index:
    // - Precondition: isValidFrameIndex(frameIndex) == true
    // - The seek position in seconds is frameIndex / samplingRate()
    // Returns the actual current frame index which may differ from the
    // requested index if the source does not support accurate seeking.
    virtual SINT seekSampleFrame(SINT frameIndex);

    // Fills the buffer with samples from each channel starting
    // at the current frame seek position.
    //
    // The implicit  minimum required capacity of the sampleBuffer is
    //     sampleBufferSize = frames2samples(numberOfFrames)
    // Samples in the sampleBuffer are stored as consecutive sample
    // frames with samples from each channel interleaved.
    //
    // Returns the actual number of frames that have been read which
    // might be lower than the requested number of frames when the end
    // of the audio stream has been reached. The current frame seek
    // position is moved forward towards the next unread frame.
    virtual SINT readSampleFrames(
            SINT numberOfFrames,
            CSAMPLE* sampleBuffer);

    inline SINT skipSampleFrames(
            SINT numberOfFrames) {
        return readSampleFrames(numberOfFrames, static_cast<CSAMPLE*>(nullptr));
    }

    inline SINT readSampleFrames(
            SINT numberOfFrames,
            SampleBuffer* pSampleBuffer) {
        if (pSampleBuffer) {
            DEBUG_ASSERT(frames2samples(numberOfFrames) <= pSampleBuffer->size());
            return readSampleFrames(numberOfFrames, pSampleBuffer->data());
        } else {
            return skipSampleFrames(numberOfFrames);
        }
    }

    ///////////////////////////////////////////////////////////////////////////


  protected:
    explicit AudioSource(const QUrl& url);
    explicit AudioSource(const AudioSource& other) = default;

    bool initFrameIndexRange(
            IndexRange frameIndexRange);

    bool initBitrate(Bitrate bitrate);
    bool initBitrate(SINT bitrate) {
        return initBitrate(Bitrate(bitrate));
    }

  private:
    friend class AudioSourceConfig;

    IndexRange m_frameIndexRange;

    Bitrate m_bitrate;
};

// Parameters for configuring audio sources
class AudioSourceConfig : public AudioSignal {
  public:
    AudioSourceConfig()
        : AudioSignal(AudioSource::sampleLayout()) {
    }
    AudioSourceConfig(ChannelCount channelCount, SamplingRate samplingRate)
        : AudioSignal(AudioSource::sampleLayout(), channelCount, samplingRate) {
    }

    using AudioSignal::setChannelCount;
    using AudioSignal::setSamplingRate;
};

} // namespace mixxx

#endif // MIXXX_AUDIOSOURCE_H
