#include "sources/audiosource.h"

#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("AudioSource");

} // anonymous namespace

AudioSource::AudioSource(QUrl url)
        : UrlResource(url),
          AudioSignal(kSampleLayout) {
}

AudioSource::OpenResult AudioSource::open(
        OpenMode mode,
        const OpenParams& params) {
    close(); // reopening is not supported

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
    VERIFY_OR_DEBUG_ASSERT(m_frameIndexRange.empty() || (m_frameIndexRange == frameIndexRange)) {
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

bool AudioSource::initBitrateOnce(Bitrate bitrate) {
    if (bitrate < Bitrate()) {
        kLogger.warning()
                << "Invalid bitrate"
                << bitrate;
        return false; // abort
    }
    VERIFY_OR_DEBUG_ASSERT(!m_bitrate.valid() || (m_bitrate == bitrate)) {
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

bool AudioSource::verifyReadable() const {
    bool result = AudioSignal::verifyReadable();
    if (frameIndexRange().empty()) {
        kLogger.warning()
                << "No audio data available";
        // Don't set the result to false, even if reading from an empty source
        // is pointless!
    }
    if (m_bitrate != Bitrate()) {
        VERIFY_OR_DEBUG_ASSERT(m_bitrate.valid()) {
            kLogger.warning()
                    << "Invalid bitrate [kbps]:"
                    << m_bitrate;
            // Don't set the result to false, because bitrate is only
            // an informational property that does not effect the ability
            // to decode audio data!
        }
    }
    return result;
}

WritableSampleFrames AudioSource::clampWritableSampleFrames(
        WritableSampleFrames sampleFrames) const {
    const auto readableFrameIndexRange =
            clampFrameIndexRange(sampleFrames.frameIndexRange());
    // adjust offset and length of the sample buffer
    DEBUG_ASSERT(sampleFrames.frameIndexRange().start() <= readableFrameIndexRange.end());
    auto writableFrameIndexRange =
            IndexRange::between(sampleFrames.frameIndexRange().start(), readableFrameIndexRange.end());
    const SINT minSampleBufferCapacity =
            frames2samples(writableFrameIndexRange.length());
    VERIFY_OR_DEBUG_ASSERT(sampleFrames.writableLength() >= minSampleBufferCapacity) {
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
                        samples2frames(sampleFrames.writableLength()));
        kLogger.warning()
                << "Reduced writable sample frames"
                << writableFrameIndexRange;
    }
    DEBUG_ASSERT(readableFrameIndexRange.start() >= writableFrameIndexRange.start());
    const SINT writableFrameOffset =
            readableFrameIndexRange.start() - writableFrameIndexRange.start();
    writableFrameIndexRange.shrinkFront(writableFrameOffset);
    return WritableSampleFrames(
            writableFrameIndexRange,
            SampleBuffer::WritableSlice(
                    sampleFrames.writableData(frames2samples(writableFrameOffset)),
                    frames2samples(writableFrameIndexRange.length())));
}

} // namespace mixxx
