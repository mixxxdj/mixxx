#pragma once

#include "sources/urlresource.h"

#include "util/audiosignal.h"
#include "util/indexrange.h"
#include "util/memory.h"
#include "util/samplebuffer.h"

namespace mixxx {

class SampleFrames {
  public:
    SampleFrames() = default;
    explicit SampleFrames(
            IndexRange frameIndexRange)
            : m_frameIndexRange(frameIndexRange) {
    }
    /*non-virtual*/ ~SampleFrames() = default;

    IndexRange frameIndexRange() const {
        return m_frameIndexRange;
    }

    SINT frameLength() const {
        return m_frameIndexRange.length();
    }

  private:
    IndexRange m_frameIndexRange;
};

// Associates a range of frame indices with the corresponding
// readable sample data as a slice in some sample buffer. The
// memory is owned by the external sample buffer that must not
// be modified or destroyed while reading sample data!
class ReadableSampleFrames final : public SampleFrames {
  public:
    ReadableSampleFrames() = default;
    explicit ReadableSampleFrames(
            IndexRange frameIndexRange,
            SampleBuffer::ReadableSlice readableSlice = SampleBuffer::ReadableSlice())
            : SampleFrames(frameIndexRange),
              m_readableSlice(readableSlice) {
    }
    /*non-virtual*/ ~ReadableSampleFrames() = default;

    // The readable slice should cover the whole range of
    // frame indices and starts with the first frame. An
    // empty slice indicates that no sample data is available
    // for reading.
    SampleBuffer::ReadableSlice readableSlice() const {
        return m_readableSlice;
    }

    SINT readableLength(SINT offset = 0) const {
        return m_readableSlice.length(offset);
    }

    const CSAMPLE* readableData(SINT offset = 0) const {
        return m_readableSlice.data(offset);
    }

  private:
    SampleBuffer::ReadableSlice m_readableSlice;
};

// Associates a range of frame indices with the corresponding
// writable sample data as a slice in some sample buffer. The
// memory is owned by the external sample buffer that must not
// be modified or destroyed while writing sample data!
class WritableSampleFrames final : public SampleFrames {
  public:
    WritableSampleFrames() = default;
    explicit WritableSampleFrames(
            IndexRange frameIndexRange,
            SampleBuffer::WritableSlice writableSlice = SampleBuffer::WritableSlice())
            : SampleFrames(frameIndexRange),
              m_writableSlice(writableSlice) {
    }
    /*non-virtual*/ ~WritableSampleFrames() = default;

    // The writable slice should cover the whole range of
    // frame indices and starts with the first frame. An
    // empty slice indicates that no sample data must
    // be written.
    SampleBuffer::WritableSlice writableSlice() const {
        return m_writableSlice;
    }

    SINT writableLength(SINT offset = 0) const {
        return m_writableSlice.length(offset);
    }

    CSAMPLE* writableData(SINT offset = 0) const {
        return m_writableSlice.data(offset);
    }

  private:
    SampleBuffer::WritableSlice m_writableSlice;
};

class IAudioSourceReader {
  public:
    virtual ~IAudioSourceReader() = default;

  protected:
    // Reads as much of the the requested sample frames and writes
    // them into the provided buffer. The capacity of the buffer
    // and the requested range have already been checked and
    // adjusted (= clamped) before if necessary.
    //
    // Returns the number of and decoded sample frames in a readable
    // buffer. The returned buffer is just a view/slice of the provided
    // writable buffer if the result is not empty. If the result is
    // empty the internal memory pointer of the returned buffer might
    // be null.
    virtual ReadableSampleFrames readSampleFramesClamped(
            WritableSampleFrames sampleFrames) = 0;

    // The following function is required for accessing the protected
    // read function from siblings implementing this interface, e.g.
    // for proxies and adapters.
    static ReadableSampleFrames readSampleFramesClampedOn(
            IAudioSourceReader& that,
            WritableSampleFrames sampleFrames) {
        return that.readSampleFramesClamped(sampleFrames);
    }
};

// Common base class for audio sources.
//
// Both the number of channels and the sample rate must
// be constant and are not allowed to change over time.
//
// Samples in a sample buffer are stored as consecutive frames,
// i.e. the samples of the channels are interleaved.
//
// Audio sources are implicitly opened upon creation and
// closed upon destruction.
class AudioSource : public UrlResource, public AudioSignal, public virtual /*implements*/ IAudioSourceReader {
  public:
    virtual ~AudioSource() = default;

    // All sources are required to produce a signal of frames
    // where each frame contains samples from all channels that are
    // coincident in time.
    //
    // A frame for a mono signal contains a single sample. A frame
    // for a stereo signal contains a pair of samples, one for the
    // left and right channel respectively.
    static constexpr SampleLayout kSampleLayout = SampleLayout::Interleaved;

    enum class OpenMode {
        // In Strict mode the opening operation should be aborted
        // as soon as any inconsistencies are detected.
        Strict,
        // Opening in Permissive mode is used only after opening
        // in Strict mode has been aborted by all available
        // SoundSource implementations.
        Permissive,
    };

    enum class OpenResult {
        Succeeded,
        // If a SoundSource is not able to open a file because of
        // internal errors of if the format of the content is not
        // supported it should return Aborted. This gives SoundSources
        // with a lower priority the chance to open the same file.
        // Example: A SoundSourceProvider has been registered for
        // files with a certain extension, but the corresponding
        // SoundSource does only support a subset of all possible
        // data formats that might be stored in files with this
        // extension.
        Aborted,
        // If a SoundSource return Failed while opening a file
        // the entire operation will fail immediately. No other
        // sources with lower priority will be given the chance
        // to open the same file.
        Failed,
    };

    // Parameters for opening audio sources
    class OpenParams : public AudioSignal {
      public:
        OpenParams()
                : AudioSignal(kSampleLayout) {
        }
        OpenParams(ChannelCount channelCount, SampleRate sampleRate)
                : AudioSignal(kSampleLayout, channelCount, sampleRate) {
        }

        using AudioSignal::setChannelCount;
        using AudioSignal::setSampleRate;
    };

    // Opens the AudioSource for reading audio data.
    //
    // Since reopening is not supported close() will be called
    // implicitly before the AudioSource is actually opened.
    //
    // Optionally the caller may provide the desired properties of
    // the decoded audio signal. Some decoders are able to reduce
    // the number of channels or do resampling efficiently on the
    // fly while decoding the input data.
    OpenResult open(
            OpenMode mode,
            const OpenParams& params = OpenParams());

    // Closes the AudioSource and frees all resources.
    //
    // Might be called even if the AudioSource has never been
    // opened, has already been closed, or if opening has failed.
    virtual void close() = 0;

    // The total length of audio data is bounded and measured in frames.
    IndexRange frameIndexRange() const {
        return m_frameIndexRange;
    }

    // The total length of audio data.
    SINT frameLength() const {
        return m_frameIndexRange.length();
    }

    // The index of the first frame.
    SINT frameIndexMin() const {
        DEBUG_ASSERT(m_frameIndexRange.start() <= m_frameIndexRange.end());
        return m_frameIndexRange.start();
    }

    // The index after the last frame.
    SINT frameIndexMax() const {
        DEBUG_ASSERT(m_frameIndexRange.start() <= m_frameIndexRange.end());
        return m_frameIndexRange.end();
    }

    // The sample frame index is valid within the range
    // [frameIndexMin(), frameIndexMax()]
    // including the upper bound of the range!
    bool isValidFrameIndex(SINT frameIndex) const {
        return m_frameIndexRange.clampIndex(frameIndex) == frameIndex;
    }

    // The actual duration in seconds.
    // Well defined only for valid files!
    inline bool hasDuration() const {
        return sampleRate().valid();
    }
    inline double getDuration() const {
        DEBUG_ASSERT(hasDuration()); // prevents division by zero
        return double(frameLength()) / double(sampleRate());
    }

    // The bitrate is optional and measured in kbit/s (kbps).
    // It depends on the metadata and decoder if a value for the
    // bitrate is available.
    class Bitrate {
      private:
        static constexpr SINT kValueDefault = 0;

      public:
        static constexpr const char* unit() {
            return "kbps";
        }

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

    ReadableSampleFrames readSampleFrames(
            WritableSampleFrames sampleFrames) {
        const auto sampleFramesFramesClamped =
                clampWritableSampleFrames(sampleFrames);
        if (sampleFramesFramesClamped.frameIndexRange().empty()) {
            // result is empty
            return ReadableSampleFrames(
                    sampleFramesFramesClamped.frameIndexRange());
        } else {
            // forward clamped request
            return readSampleFramesClamped(
                    sampleFramesFramesClamped);
        }
    }

  protected:
    explicit AudioSource(QUrl url);
    AudioSource(const AudioSource&) = default;

    bool initFrameIndexRangeOnce(
            IndexRange frameIndexRange);

    bool initBitrateOnce(Bitrate bitrate);
    bool initBitrateOnce(SINT bitrate) {
        return initBitrateOnce(Bitrate(bitrate));
    }

    // Tries to open the AudioSource for reading audio data according
    // to the "Template Method" design pattern.
    //
    // The invocation of tryOpen() is enclosed in invocations of close():
    //   - Before: Always
    //   - After: Upon failure
    // If tryOpen() throws an exception or returns a result other than
    // OpenResult::Succeeded an invocation of close() will follow.
    // Implementations do not need to free internal resources twice in
    // both tryOpen() upon failure and close(). All internal resources
    // should be freed in close() instead.
    //
    // Exceptions should be handled internally by implementations to
    // avoid warning messages about unexpected or unknown exceptions.
    virtual OpenResult tryOpen(
            OpenMode mode,
            const OpenParams& params) = 0;

    static OpenResult tryOpenOn(
            AudioSource& that,
            OpenMode mode,
            const OpenParams& params) {
        return that.open(mode, params);
    }

  private:
    AudioSource(AudioSource&&) = delete;
    AudioSource& operator=(const AudioSource&) = delete;
    AudioSource& operator=(AudioSource&&) = delete;

    WritableSampleFrames clampWritableSampleFrames(
            WritableSampleFrames sampleFrames) const;
    IndexRange clampFrameIndexRange(
            IndexRange frameIndexRange) const {
        return intersect(frameIndexRange, this->frameIndexRange());
    }

    IndexRange m_frameIndexRange;

    Bitrate m_bitrate;
};

typedef std::shared_ptr<AudioSource> AudioSourcePointer;

inline QDebug operator<<(QDebug dbg, AudioSource::OpenMode openMode) {
    switch (openMode) {
    case AudioSource::OpenMode::Strict:
        return dbg << "Strict";
    case AudioSource::OpenMode::Permissive:
        return dbg << "Permissive";
    default:
        DEBUG_ASSERT(!"Unknown OpenMode");
        return dbg << "Unknown";
    }
}

inline QDebug operator<<(QDebug dbg, AudioSource::OpenResult openResult) {
    switch (openResult) {
    case AudioSource::OpenResult::Succeeded:
        return dbg << "Succeeded";
    case AudioSource::OpenResult::Aborted:
        return dbg << "Aborted";
    case AudioSource::OpenResult::Failed:
        return dbg << "Failed";
    default:
        DEBUG_ASSERT(!"Unknown OpenResult");
        return dbg << "Unknown";
    }
}

} // namespace mixxx
