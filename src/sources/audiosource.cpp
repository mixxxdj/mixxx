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


IndexRange AudioSource::adjustReadableFrameIndexRangeAndOutputBuffer(
        IndexRange frameIndexRange,
        SampleBuffer::WritableSlice* pOutputBuffer) const {
    const SINT minOutputBufferSize =
            frames2samples(frameIndexRange.length());
    VERIFY_OR_DEBUG_ASSERT(!pOutputBuffer ||
            (pOutputBuffer->size() >= minOutputBufferSize)) {
        kLogger.critical()
                << "Output buffer is too small"
                << pOutputBuffer->size()
                << "<"
                << minOutputBufferSize
                << "to store all sample frames"
                << frameIndexRange;
        return IndexRange();
    }

    auto readableFrames =
            intersect(frameIndexRange, this->frameIndexRange());
    if (readableFrames.empty()) {
        return readableFrames;
    }
    DEBUG_ASSERT(readableFrames.head() >= frameIndexRange.head());
    if (pOutputBuffer) {
        const SINT readableFrameOffset = readableFrames.head() - frameIndexRange.head();
        if (readableFrameOffset > 0) {
            *pOutputBuffer = SampleBuffer::WritableSlice(
                    pOutputBuffer->data(frames2samples(readableFrameOffset)),
                    frames2samples(readableFrames.length()));
        }
    }
    return readableFrames;
}

} // namespace mixxx
