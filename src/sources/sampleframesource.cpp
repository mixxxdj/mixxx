#include "sources/sampleframesource.h"

#include "util/logger.h"


namespace mixxx {

namespace {

const Logger kLogger("SampleFrameSource");

} // anonymous namespace

bool SampleFrameSource::initFrameIndexRangeOnce(
        IndexRange frameIndexRange) {
    VERIFY_OR_DEBUG_ASSERT(frameIndexRange.orientation() != mixxx::IndexRange::Orientation::Backward) {
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

bool SampleFrameSource::verifyReadable() const {
    bool result = AudioSignal::verifyReadable();
    if (frameIndexRange().empty()) {
        kLogger.warning()
                << "No audio data available";
        // Don't set the result to false, even if reading from an empty source
        // is pointless!
    }
    return result;
}

WritableSampleFrames SampleFrameSource::clampWritableSampleFrames(
        ReadMode readMode,
        WritableSampleFrames sampleFrames) const {
    const auto readableFrameIndexRange =
            clampFrameIndexRange(sampleFrames.frameIndexRange());
    if ((readMode == ReadMode::Skip) ||
            readableFrameIndexRange.empty() ||
            sampleFrames.sampleBuffer().empty()) {
        // should not write any samples into buffer
        return WritableSampleFrames(readableFrameIndexRange);
    } else {
        // adjust offset and length of the sample buffer
        DEBUG_ASSERT(sampleFrames.frameIndexRange().start() <= readableFrameIndexRange.end());
        auto writableFrameIndexRange =
                IndexRange::between(sampleFrames.frameIndexRange().start(), readableFrameIndexRange.end());
        const SINT minSampleBufferCapacity =
                frames2samples(writableFrameIndexRange.length());
        VERIFY_OR_DEBUG_ASSERT(sampleFrames.sampleBuffer().size() >= minSampleBufferCapacity) {
            kLogger.critical()
                    << "Capacity of output buffer is too small"
                    << sampleFrames.sampleBuffer().size()
                    << "<"
                    << minSampleBufferCapacity
                    << "to store all readable sample frames"
                    << readableFrameIndexRange
                    << "into writable sample frames"
                    << writableFrameIndexRange;
            writableFrameIndexRange =
                    writableFrameIndexRange.cutFrontRange(
                            samples2frames(sampleFrames.sampleBuffer().size()));
            kLogger.warning()
                    << "Reduced writable sample frames"
                    << writableFrameIndexRange;
        }
        DEBUG_ASSERT(readableFrameIndexRange.start() >= writableFrameIndexRange.start());
        const SINT writableFrameOffset =
                readableFrameIndexRange.start() - writableFrameIndexRange.start();
        writableFrameIndexRange.dropFrontRange(writableFrameOffset);
        return WritableSampleFrames(
                writableFrameIndexRange,
                SampleBuffer::WritableSlice(
                        sampleFrames.sampleBuffer().data(frames2samples(writableFrameOffset)),
                        frames2samples(writableFrameIndexRange.length())));
    }
}

} // namespace mixxx
