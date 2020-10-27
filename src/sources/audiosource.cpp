#include "sources/audiosource.h"

#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("AudioSource");

// Maximum number of sample frames to verify that decoding the audio
// stream works.
// NOTE(2020-05-01): A single frame is sufficient to reliably detect
// the broken FAAD2 v2.9.1 library.
const SINT kVerifyReadableMaxFrameCount = 1;

} // anonymous namespace

AudioSource::AudioSource(QUrl url)
        : UrlResource(url),
          m_signalInfo(kSampleLayout) {
}

AudioSource::AudioSource(
        const AudioSource& inner,
        const audio::SignalInfo& signalInfo)
        : UrlResource(inner),
          m_signalInfo(signalInfo),
          m_bitrate(inner.m_bitrate),
          m_frameIndexRange(inner.m_frameIndexRange) {
}

AudioSource::OpenResult AudioSource::open(
        OpenMode mode,
        const OpenParams& params) {
    close(); // reopening is not supported
    DEBUG_ASSERT(!getSignalInfo().isValid());

    OpenResult result;
    try {
        result = tryOpen(mode, params);
    } catch (const std::exception& e) {
        qWarning() << "Caught unexpected exception from SoundSource::tryOpen():" << e.what();
        result = OpenResult::Failed;
    } catch (...) {
        qWarning() << "Caught unknown exception from SoundSource::tryOpen()";
        result = OpenResult::Failed;
    }
    if (OpenResult::Succeeded != result) {
        close(); // rollback
    }
    return result;
}

bool AudioSource::initFrameIndexRangeOnce(
        IndexRange frameIndexRange) {
    VERIFY_OR_DEBUG_ASSERT(frameIndexRange.orientation() != IndexRange::Orientation::Backward) {
        kLogger.warning()
                << "Backward frame index range not supported"
                << frameIndexRange;
        return false; // abort
    }
    if (!m_frameIndexRange.empty() &&
            m_frameIndexRange != frameIndexRange) {
        kLogger.warning()
                << "Frame index range has already been initialized to"
                << m_frameIndexRange
                << "which differs from"
                << frameIndexRange;
        return false; // abort
    }
    m_frameIndexRange = frameIndexRange;
    return true;
}

bool AudioSource::initChannelCountOnce(
        audio::ChannelCount channelCount) {
    if (!channelCount.isValid()) {
        kLogger.warning()
                << "Invalid channel count"
                << channelCount;
        return false; // abort
    }
    if (m_signalInfo.getChannelCount().isValid() &&
            m_signalInfo.getChannelCount() != channelCount) {
        kLogger.warning()
                << "Channel count has already been initialized to"
                << m_signalInfo.getChannelCount()
                << "which differs from"
                << channelCount;
        return false; // abort
    }
    m_signalInfo.setChannelCount(channelCount);
    return true;
}

bool AudioSource::initSampleRateOnce(
        audio::SampleRate sampleRate) {
    if (!sampleRate.isValid()) {
        kLogger.warning()
                << "Invalid sample rate"
                << sampleRate;
        return false; // abort
    }
    if (m_signalInfo.getSampleRate().isValid() &&
            m_signalInfo.getSampleRate() != sampleRate) {
        kLogger.warning()
                << "Sample rate has already been initialized to"
                << m_signalInfo.getSampleRate()
                << "which differs from"
                << sampleRate;
        return false; // abort
    }
    m_signalInfo.setSampleRate(sampleRate);
    return true;
}

bool AudioSource::initBitrateOnce(audio::Bitrate bitrate) {
    // Bitrate is optional and might be invalid (= audio::Bitrate())
    if (bitrate < audio::Bitrate()) {
        kLogger.warning()
                << "Invalid bitrate"
                << bitrate;
        return false; // abort
    }
    VERIFY_OR_DEBUG_ASSERT(
            !m_bitrate.isValid() ||
            m_bitrate == bitrate) {
        kLogger.warning()
                << "Bitrate has already been initialized to"
                << m_bitrate
                << "which differs from"
                << bitrate;
        return false; // abort
    }
    m_bitrate = bitrate;
    return true;
}

bool AudioSource::verifyReadable() {
    // No early return desired! All tests should be performed, even
    // if some fail.
    bool result = true;
    DEBUG_ASSERT(m_signalInfo.getSampleLayout());
    if (!m_signalInfo.getChannelCount().isValid()) {
        kLogger.warning()
                << "Invalid number of channels:"
                << getSignalInfo().getChannelCount()
                << "is out of range ["
                << audio::ChannelCount::min()
                << ","
                << audio::ChannelCount::max()
                << "]";
        result = false;
    }
    if (!m_signalInfo.getSampleRate().isValid()) {
        kLogger.warning()
                << "Invalid sample rate:"
                << getSignalInfo().getSampleRate()
                << "is out of range ["
                << audio::SampleRate::min()
                << ","
                << audio::SampleRate::max()
                << "]";
        result = false;
    }
    DEBUG_ASSERT(result == m_signalInfo.isValid());
    // Bitrate is optional and might be invalid (= audio::Bitrate())
    if (m_bitrate != audio::Bitrate()) {
        // Non-default bitrate must be valid
        VERIFY_OR_DEBUG_ASSERT(m_bitrate.isValid()) {
            kLogger.warning()
                    << "Invalid bitrate"
                    << m_bitrate;
            // Don't set the result to false, because bitrate is only
            // an informational property that does not effect the ability
            // to decode audio data!
        }
    }
    if (!result) {
        // Invalid or inconsistent properties detected. We can abort
        // at this point and do not need to perform any read tests.
        return false;
    }
    if (frameIndexRange().empty()) {
        kLogger.warning()
                << "No audio data available, i.e. stream is empty";
        // Don't return false, even if reading from an empty source
        // is pointless. It is still a valid audio stream.
        return true;
    }
    // Try to read some test frames to ensure that decoding actually works!
    //
    // Counterexample: The broken FAAD version 2.9.1 is able to open a file
    // but then fails to decode any sample frames.
    const SINT numSampleFrames =
            math_min(kVerifyReadableMaxFrameCount, frameIndexRange().length());
    SampleBuffer sampleBuffer(
            m_signalInfo.frames2samples(numSampleFrames));
    WritableSampleFrames writableSampleFrames(
            frameIndexRange().splitAndShrinkFront(numSampleFrames),
            SampleBuffer::WritableSlice(sampleBuffer));
    auto readableSampleFrames = readSampleFrames(writableSampleFrames);
    DEBUG_ASSERT(
            readableSampleFrames.frameIndexRange() <=
            writableSampleFrames.frameIndexRange());
    if (readableSampleFrames.frameIndexRange() <
            writableSampleFrames.frameIndexRange()) {
        kLogger.warning()
                << "Read test failed:"
                << "expected ="
                << writableSampleFrames.frameIndexRange()
                << ", actual ="
                << readableSampleFrames.frameIndexRange();
        return false;
    }
    return true;
}

std::optional<WritableSampleFrames> AudioSource::clampWritableSampleFrames(
        WritableSampleFrames sampleFrames) const {
    const auto clampedFrameIndexRange =
            clampFrameIndexRange(sampleFrames.frameIndexRange());
    if (!clampedFrameIndexRange) {
        return std::nullopt;
    }
    const auto readableFrameIndexRange = *clampedFrameIndexRange;

    // adjust offset and length of the sample buffer
    DEBUG_ASSERT(
            sampleFrames.frameIndexRange().start() <=
            readableFrameIndexRange.end());
    auto writableFrameIndexRange =
            IndexRange::between(
                    sampleFrames.frameIndexRange().start(),
                    readableFrameIndexRange.end());
    const SINT minSampleBufferCapacity =
            m_signalInfo.frames2samples(
                    writableFrameIndexRange.length());
    VERIFY_OR_DEBUG_ASSERT(
            sampleFrames.writableLength() >=
            minSampleBufferCapacity) {
        kLogger.critical()
                << "Capacity of output buffer is too small"
                << sampleFrames.writableLength()
                << "<"
                << minSampleBufferCapacity
                << "to store all readable sample frames"
                << readableFrameIndexRange
                << "into writable sample frames"
                << writableFrameIndexRange;
        writableFrameIndexRange =
                writableFrameIndexRange.splitAndShrinkFront(
                        m_signalInfo.samples2frames(
                                sampleFrames.writableLength()));
        kLogger.warning()
                << "Reduced writable sample frames"
                << writableFrameIndexRange;
    }
    DEBUG_ASSERT(
            readableFrameIndexRange.start() >=
            writableFrameIndexRange.start());
    const SINT writableFrameOffset =
            readableFrameIndexRange.start() -
            writableFrameIndexRange.start();
    writableFrameIndexRange.shrinkFront(
            writableFrameOffset);
    return WritableSampleFrames(
            writableFrameIndexRange,
            SampleBuffer::WritableSlice(
                    sampleFrames.writableData(
                            m_signalInfo.frames2samples(writableFrameOffset)),
                    m_signalInfo.frames2samples(
                            writableFrameIndexRange.length())));
}

ReadableSampleFrames AudioSource::readSampleFrames(
        WritableSampleFrames sampleFrames) {
    const auto clamped =
            clampWritableSampleFrames(sampleFrames);
    if (!clamped) {
        // result is undefined
        // TODO: Changing the return type to std::optional<ReadableSampleFrames>
        // and instead returning std::nullopt here would be more appropriate
        return ReadableSampleFrames(
                IndexRange::forward(sampleFrames.frameIndexRange().start(), 0));
    }
    const auto writable = *clamped;
    if (writable.frameIndexRange().empty()) {
        // result is empty
        return ReadableSampleFrames(writable.frameIndexRange());
    } else {
        // forward clamped request
        ReadableSampleFrames readable = readSampleFramesClamped(writable);
        DEBUG_ASSERT(readable.frameIndexRange().empty() ||
                readable.frameIndexRange() <= writable.frameIndexRange());
        if (readable.frameIndexRange() != writable.frameIndexRange()) {
            kLogger.warning()
                    << "Failed to read sample frames:"
                    << "expected =" << writable.frameIndexRange()
                    << ", actual =" << readable.frameIndexRange();
            auto shrinkedFrameIndexRange = m_frameIndexRange;
            if (readable.frameIndexRange().empty()) {
                // Adjust upper bound: Consider all audio data following
                // the read position until the end as unreadable
                shrinkedFrameIndexRange.shrinkBack(
                        shrinkedFrameIndexRange.end() -
                        writable.frameIndexRange().start());
            } else {
                // Adjust lower bound of readable audio data
                if (writable.frameIndexRange().start() <
                        readable.frameIndexRange().start()) {
                    shrinkedFrameIndexRange.shrinkFront(
                            readable.frameIndexRange().start() -
                            shrinkedFrameIndexRange.start());
                }
                // Adjust upper bound of readable audio data
                if (writable.frameIndexRange().end() >
                        readable.frameIndexRange().end()) {
                    shrinkedFrameIndexRange.shrinkBack(
                            shrinkedFrameIndexRange.end() -
                            readable.frameIndexRange().end());
                }
            }
            DEBUG_ASSERT(shrinkedFrameIndexRange < m_frameIndexRange);
            kLogger.info()
                    << "Shrinking readable frame index range:"
                    << "before =" << m_frameIndexRange
                    << ", after =" << shrinkedFrameIndexRange;
            // Propagate the adjustments to all participants in the
            // inheritance hierarchy.
            // NOTE(2019-08-31, uklotzde): This is an ugly hack to overcome
            // the previous assumption that the frame index range is immutable
            // for the whole lifetime of an AudioSource. As we know now it is
            // not and for a future re-design we need to account for this fact!!
            adjustFrameIndexRange(shrinkedFrameIndexRange);
        }
        return readable;
    }
}

void AudioSource::adjustFrameIndexRange(
        IndexRange frameIndexRange) {
    DEBUG_ASSERT(frameIndexRange <= m_frameIndexRange);
    m_frameIndexRange = frameIndexRange;
}

} // namespace mixxx
