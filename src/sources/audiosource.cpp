#include "sources/audiosource.h"

#include "util/sample.h"
#include "util/logger.h"


namespace mixxx {

namespace {

const Logger kLogger("AudioSource");

} // anonymous namespace

AudioSource::AudioSource(const QUrl& url)
        : UrlResource(url),
          AudioSignal(sampleLayout()) {
}

bool AudioSource::initFrameIndexRange(
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

bool AudioSource::initBitrate(Bitrate bitrate) {
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

IndexRange AudioSource::readOrSkipSampleFrames(
        IndexRange frameIndexRange,
        SampleBuffer::WritableSlice* pOutputBuffer) {
    // Default implementation based on the v1 legacy interface
    // TODO(XXX): Delete this implementation and declare the function
    // as pure virtual after functions of the v1 legacy interface have
    // been deleted.
    VERIFY_OR_DEBUG_ASSERT(frameIndexRange.orientation() != mixxx::IndexRange::Orientation::Backward) {
        kLogger.warning()
                << "Backward frame index range not supported"
                << frameIndexRange;
        return IndexRange(); // abort
    }
    VERIFY_OR_DEBUG_ASSERT(!pOutputBuffer ||
            (pOutputBuffer->size() >= frames2samples(frameIndexRange.length()))) {
        kLogger.warning()
                << "Read request exceeds size of output buffer:"
                << frames2samples(frameIndexRange.length())
                << ">"
                << pOutputBuffer->size();
        return IndexRange();
    }

    auto readableFrameIndexRange =
            intersect(frameIndexRange, this->frameIndexRange());
    if (readableFrameIndexRange.empty()) {
        return readableFrameIndexRange;
    }
    DEBUG_ASSERT(frameIndexRange.head() <= readableFrameIndexRange.head());

    SINT seekFrameIndex = seekSampleFrame(readableFrameIndexRange.head());
    if (pOutputBuffer && (seekFrameIndex < readableFrameIndexRange.head())) {
        // Fallback: Try to skip frames up to the first readable frame index
        const auto skipFrameIndexRange =
                IndexRange::between(seekFrameIndex, readableFrameIndexRange.head());
        const auto skippedFrameIndexRange =
                skipSampleFrames(skipFrameIndexRange);
        if (skipFrameIndexRange != skippedFrameIndexRange) {
            kLogger.warning()
                    << "Failed to start reading sample frames";
            return IndexRange();
        }
        seekFrameIndex = skippedFrameIndexRange.tail();
    }
    DEBUG_ASSERT(readableFrameIndexRange.head() <= seekFrameIndex);
    // Skip unreadable frames up to seek position
    readableFrameIndexRange.dropHead(seekFrameIndex - readableFrameIndexRange.head());
    DEBUG_ASSERT(readableFrameIndexRange.head() == seekFrameIndex);

    if (pOutputBuffer) {
        // Skip unreadable sample frames in output buffer
        SINT outputBufferDataOffset = 0;
        if (frameIndexRange.head() < readableFrameIndexRange.head()) {
            const auto unreadableFrameIndexRange =
                    IndexRange::between(frameIndexRange.head(), readableFrameIndexRange.head());
            outputBufferDataOffset +=
                    frames2samples(unreadableFrameIndexRange.length());
            DEBUG_ASSERT(outputBufferDataOffset <= pOutputBuffer->size());
        }
        // Read data
        return IndexRange::forward(
                readableFrameIndexRange.head(),
                readSampleFrames(
                        readableFrameIndexRange.length(),
                        pOutputBuffer->data() + outputBufferDataOffset));
    } else {
        // Skip data
        return IndexRange::forward(
                readableFrameIndexRange.head(),
                skipSampleFrames(
                        readableFrameIndexRange.length()));
    }
    DEBUG_ASSERT(!"unreachable code");
    return IndexRange();
}

bool AudioSource::verifyReadable() const {
    bool result = AudioSignal::verifyReadable();
    if (m_frameIndexRange.empty()) {
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
            // an  informational property that does not effect the ability
            // to decode audio data!
        }
    }
    return result;
}

SINT AudioSource::seekSampleFrame(SINT frameIndex) {
    DEBUG_ASSERT(!"not implemented");
    kLogger.critical()
            << "seekSampleFrame() not implemented";
    return frameIndex;
}

SINT AudioSource::readSampleFrames(
        SINT /*numberOfFrames*/,
        CSAMPLE* /*sampleBuffer*/) {
    DEBUG_ASSERT(!"not implemented");
    kLogger.critical()
            << "readSampleFrames() not implemented";
    return 0;
}

} // namespace mixxx
