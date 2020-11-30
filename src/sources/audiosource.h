#pragma once

#include "audio/streaminfo.h"
#include "engine/engine.h"
#include "sources/urlresource.h"
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
            const WritableSampleFrames& sampleFrames) = 0;

    // The following function is required for accessing the protected
    // read function from siblings implementing this interface, e.g.
    // for proxies and adapters.
    static ReadableSampleFrames readSampleFramesClampedOn(
            IAudioSourceReader& that,
            const WritableSampleFrames& sampleFrames) {
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
class AudioSource : public UrlResource, public virtual /*implements*/ IAudioSourceReader {
  public:
    ~AudioSource() override = default;

    // All sources are required to produce a signal of frames
    // where each frame contains samples from all channels that are
    // coincident in time.
    //
    // A frame for a mono signal contains a single sample. A frame
    // for a stereo signal contains a pair of samples, one for the
    // left and right channel respectively.
    static constexpr audio::SampleLayout kSampleLayout = mixxx::kEngineSampleLayout;

    /// Defines how thoroughly the stream properties should be verified
    /// when opening an audio stream.
    enum class OpenMode {
        /// In Strict mode the opening operation should be aborted
        /// as soon as any inconsistencies of the stream properties
        /// are detected when setting up the decoding.
        Strict,

        /// Opening in Permissive mode is used only after opening
        /// in Strict mode has been aborted by all available
        /// SoundSource implementations.
        ///
        /// Example: Assume safe default values if mandatory stream
        /// properties could not be determined when setting up the
        /// decoding.
        Permissive,
    };

    /// Result of opening an audio stream.
    enum class OpenResult {
        /// The file has been opened successfully and should be readable.
        Succeeded,

        /// If a SoundSource is not able to open a file because of
        /// internal errors of if the format of the content is not
        /// supported it should return Aborted.
        ///
        /// Returing this error result gives other decoders with a
        /// lower priority the chance to open the same file.
        /// Example: A SoundSourceProvider has been registered for
        /// files with a certain extension, but the corresponding
        /// SoundSource does only support a subset of all possible
        /// data formats that might be stored in files with this
        /// extension.

        Aborted,
        /// If a SoundSource return Failed while opening a file the
        /// file itself is supposed to be corrupt.
        ///
        /// If this happens during OpenMode::Strict then other decoders
        /// with a lower priority are still given a chance to try
        /// opening the file. In OpenMode::Permissive the first
        /// SoundSource that returns Failed will declare this file
        /// corrupt.
        Failed,
    };

    // Parameters for opening audio sources
    class OpenParams {
      public:
        OpenParams()
                : m_signalInfo(kSampleLayout) {
        }
        OpenParams(
                audio::ChannelCount channelCount,
                audio::SampleRate sampleRate)
                : m_signalInfo(
                          channelCount,
                          sampleRate,
                          kSampleLayout) {
        }

        const audio::SignalInfo& getSignalInfo() const {
            return m_signalInfo;
        }

        void setChannelCount(
                audio::ChannelCount channelCount) {
            m_signalInfo.setChannelCount(channelCount);
        }

        void setSampleRate(
                audio::SampleRate sampleRate) {
            m_signalInfo.setSampleRate(sampleRate);
        }

      private:
        audio::SignalInfo m_signalInfo;
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

    const audio::SignalInfo& getSignalInfo() const {
        return m_signalInfo;
    }

    const audio::Bitrate getBitrate() const {
        return m_bitrate;
    }

    audio::StreamInfo getStreamInfo() const {
        return audio::StreamInfo(
                getSignalInfo(),
                getBitrate(),
                Duration::fromSeconds(getDuration()));
    }

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
        return getSignalInfo().getSampleRate().isValid();
    }
    inline double getDuration() const {
        DEBUG_ASSERT(hasDuration()); // prevents division by zero
        return getSignalInfo().frames2secs(frameLength());
    }

    /// Verifies various properties to ensure that the audio data is
    /// actually readable. Warning messages are logged for properties
    /// with invalid values for diagnostic purposes. Also performs a
    /// basic read test.
    bool verifyReadable();

    ReadableSampleFrames readSampleFrames(
            const WritableSampleFrames& sampleFrames);

  protected:
    explicit AudioSource(const QUrl& url);

    bool initChannelCountOnce(audio::ChannelCount channelCount);
    bool initChannelCountOnce(SINT channelCount) {
        return initChannelCountOnce(audio::ChannelCount(channelCount));
    }

    bool initSampleRateOnce(audio::SampleRate sampleRate);
    bool initSampleRateOnce(SINT sampleRate) {
        return initSampleRateOnce(audio::SampleRate(sampleRate));
    }

    bool initBitrateOnce(audio::Bitrate bitrate);
    bool initBitrateOnce(SINT bitrate) {
        return initBitrateOnce(audio::Bitrate(bitrate));
    }

    bool initFrameIndexRangeOnce(
            IndexRange frameIndexRange);
    // The frame index range needs to be adjusted while
    // reading. This virtual function is an ugly hack!!!
    // It needs to be overridden in derived proxy classes
    // that wrap a pointer to the actual audio source and
    // delegate to that.
    virtual void adjustFrameIndexRange(
            IndexRange frameIndexRange);
    static void adjustFrameIndexRangeOn(
            AudioSource& that,
            IndexRange frameIndexRange) {
        that.adjustFrameIndexRange(frameIndexRange);
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
    AudioSource(const AudioSource&) = delete;
    AudioSource(AudioSource&&) = delete;
    AudioSource& operator=(const AudioSource&) = delete;
    AudioSource& operator=(AudioSource&&) = delete;

    // Ugly workaround for AudioSourceProxy to wrap
    // an existing AudioSource.
    friend class AudioSourceProxy;
    AudioSource(
            const AudioSource& inner,
            const audio::SignalInfo& signalInfo);

    std::optional<WritableSampleFrames> clampWritableSampleFrames(
            const WritableSampleFrames& sampleFrames) const;

    audio::SignalInfo m_signalInfo;

    audio::Bitrate m_bitrate;

    IndexRange m_frameIndexRange;
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
