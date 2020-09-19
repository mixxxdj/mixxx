#include "sources/readaheadframebuffer.h"

#include "util/logger.h"
#include "util/sample.h"

namespace mixxx {

namespace {

const Logger kLogger("ReadAheadFrameBuffer");

} // anonymous namespace

ReadAheadFrameBuffer::ReadAheadFrameBuffer()
        : ReadAheadFrameBuffer(audio::SignalInfo(), 0) {
}

ReadAheadFrameBuffer::ReadAheadFrameBuffer(
        const audio::SignalInfo& signalInfo,
        FrameCount initialFrameCapacity)
        : m_signalInfo(signalInfo),
          m_sampleBuffer(
                  signalInfo.isValid()
                          ? m_signalInfo.frames2samples(
                                    math_max(initialFrameCapacity, kMinFrameCapacity))
                          : 0),
          m_firstFrameIndex(kUnknownFrameIndex) {
}

void ReadAheadFrameBuffer::reinit(
        const audio::SignalInfo& signalInfo,
        FrameCount minFrameCapacity) {
    DEBUG_ASSERT(signalInfo.isValid());
    m_signalInfo = signalInfo;
    reset();
    ensureMinFrameCapacity(minFrameCapacity);
}

void ReadAheadFrameBuffer::ensureMinFrameCapacity(FrameCount minFrameCapacity) {
    DEBUG_ASSERT(minFrameCapacity >= 0);
    minFrameCapacity = math_max(minFrameCapacity, kMinFrameCapacity);
    if (frameCapacity() < minFrameCapacity) {
        m_sampleBuffer.adjustCapacity(
                m_signalInfo.frames2samples(minFrameCapacity));
    }
}

bool ReadAheadFrameBuffer::trySeekToFirstFrame(FrameIndex firstFrameIndex) {
    DEBUG_ASSERT(isReady());
    const auto clampedFrameIndex = bufferedFrameRange().clampIndex(firstFrameIndex);
    if (clampedFrameIndex != firstFrameIndex) {
        return false;
    }
    DEBUG_ASSERT(clampedFrameIndex >= m_firstFrameIndex);
    discardFirstBufferedFrames(clampedFrameIndex - m_firstFrameIndex);
    return true;
}

FrameCount ReadAheadFrameBuffer::discardFirstBufferedFrames(FrameCount frameCount) {
    DEBUG_ASSERT(isReady());
    DEBUG_ASSERT(frameCount >= 0);
    const auto shrinkCount = m_signalInfo.samples2frames(
            m_sampleBuffer.shrinkForReading(
                                  m_signalInfo.frames2samples(frameCount))
                    .length());
    m_firstFrameIndex += shrinkCount;
    return shrinkCount;
}

FrameCount ReadAheadFrameBuffer::discardLastBufferedFrames(FrameCount frameCount) {
    DEBUG_ASSERT(isReady());
    DEBUG_ASSERT(frameCount >= 0);
    return m_sampleBuffer.shrinkAfterWriting(
            m_signalInfo.frames2samples(frameCount));
}

ReadableSampleFrames ReadAheadFrameBuffer::bufferFrames(
        BufferingMode bufferingMode,
        ReadableSampleFrames readableSampleFrames) {
    DEBUG_ASSERT(isValid());
    auto sampleFrameRange = readableSampleFrames.frameIndexRange();
    VERIFY_OR_DEBUG_ASSERT(sampleFrameRange.orientation() != IndexRange::Orientation::Backward) {
        return readableSampleFrames;
    }
    FrameIndex nextFrameIndex;
    if (isReady()) {
        nextFrameIndex = bufferedFrameRange().end();
        VERIFY_OR_DEBUG_ASSERT(nextFrameIndex >= bufferedFrameRange().end()) {
            kLogger.warning()
                    << "Cannot buffer sample data from"
                    << sampleFrameRange
                    << "starting before"
                    << bufferedFrameRange().end();
            return readableSampleFrames;
        }
    } else {
        DEBUG_ASSERT(isEmpty());
        nextFrameIndex = sampleFrameRange.start();
        m_firstFrameIndex = nextFrameIndex;
    }
    DEBUG_ASSERT(isReady());
    DEBUG_ASSERT(nextFrameIndex <= sampleFrameRange.start());
    const auto writableSampleLength =
            m_signalInfo.frames2samples(sampleFrameRange.end() - nextFrameIndex);
    if (writableSampleLength > m_sampleBuffer.writableLength()) {
        // Increasing the pre-allocated capacity of the sample buffer should
        // never happen. The required capacity should always be known and
        // allocated in advance.
        const auto sampleBufferCapacity =
                m_sampleBuffer.readableLength() +
                writableSampleLength;
        kLogger.warning()
                << "Increasing capacity of internal sample buffer by reallocation:"
                << m_sampleBuffer.capacity()
                << "->"
                << sampleBufferCapacity;
        m_sampleBuffer.adjustCapacity(sampleBufferCapacity);
    }
    DEBUG_ASSERT(m_sampleBuffer.writableLength() >= writableSampleLength);
    if (nextFrameIndex < sampleFrameRange.start()) {
        // Gap between current position and the first frame with readable samples
        switch (bufferingMode) {
        case BufferingMode::SkipGap:
            break;
        case BufferingMode::FillGapWithSilence: {
#if VERBOSE_DEBUG_LOG
            kLogger.debug()
                    << "Filling gap with silence"
                    << IndexRange::between(nextFrameIndex, sampleFrameRange.start());
#endif
            const auto clearFrameCount =
                    sampleFrameRange.start() - nextFrameIndex;
            const auto clearSampleCount =
                    m_signalInfo.frames2samples(clearFrameCount);
            const SampleBuffer::WritableSlice writableSlice(
                    m_sampleBuffer.growForWriting(clearSampleCount));
            DEBUG_ASSERT(writableSlice.length() == clearSampleCount);
            SampleUtil::clear(
                    writableSlice.data(),
                    clearSampleCount);
        } break;
        default:
            DEBUG_ASSERT(!"unexpected BufferNextFramesMode");
        }
        nextFrameIndex = sampleFrameRange.start();
    }
    if (sampleFrameRange.empty()) {
        return readableSampleFrames;
    }
    // Consume the readable sample data by copying it into the internal buffer
#if VERBOSE_DEBUG_LOG
    kLogger.debug()
            << "Buffering sample frames"
            << sampleFrameRange;
#endif
    const auto copySampleCount =
            m_signalInfo.frames2samples(sampleFrameRange.length());
    const SampleBuffer::WritableSlice writableSlice(
            m_sampleBuffer.growForWriting(copySampleCount));
    DEBUG_ASSERT(writableSlice.length() == copySampleCount);
    SampleUtil::copy(
            writableSlice.data(),
            readableSampleFrames.readableData(),
            copySampleCount);
    sampleFrameRange.shrinkFront(sampleFrameRange.length());
    return ReadableSampleFrames(sampleFrameRange);
}

WritableSampleFrames ReadAheadFrameBuffer::consumeBufferedFrames(
        WritableSampleFrames writableSampleFrames) {
    if (isEmpty() || writableSampleFrames.writableSlice().empty()) {
        return writableSampleFrames;
    }
    DEBUG_ASSERT(isReady());
    auto writableFrameRange = writableSampleFrames.frameIndexRange();
    const auto consumableFrameRange = intersect(bufferedFrameRange(), writableFrameRange);
    DEBUG_ASSERT(consumableFrameRange <= writableFrameRange);
    if (consumableFrameRange.empty() ||
            (consumableFrameRange.start() != writableFrameRange.start())) {
        return writableSampleFrames;
    }

    // Drop and skip any buffered samples precedubg the requested range
    DEBUG_ASSERT(consumableFrameRange.start() >= m_firstFrameIndex);
#if VERBOSE_DEBUG_LOG
    if (m_firstFrameIndex < consumableFrameRange.start()) {
        kLogger.debug()
                << "Discarding buffered frames"
                << IndexRange::between(m_firstFrameIndex, consumableFrameRange.start());
    }
#endif
    discardFirstBufferedFrames(consumableFrameRange.start() - m_firstFrameIndex);

    // Consume buffered samples
#if VERBOSE_DEBUG_LOG
    kLogger.debug()
            << "Consuming buffered samples" << consumableFrameRange
            << "writableFrameRange" << writableFrameRange
            << "firstFrame()" << firstFrame()
            << "bufferedFrameRange()" << bufferedFrameRange();
#endif
    DEBUG_ASSERT(m_firstFrameIndex == consumableFrameRange.start());
    const SampleBuffer::ReadableSlice consumableSlice =
            m_sampleBuffer.shrinkForReading(
                    m_signalInfo.frames2samples(consumableFrameRange.length()));
    DEBUG_ASSERT(consumableSlice.length() ==
            m_signalInfo.frames2samples(consumableFrameRange.length()));
    CSAMPLE* pOutputSampleBuffer = writableSampleFrames.writableData();
    if (pOutputSampleBuffer) {
        SampleUtil::copy(pOutputSampleBuffer,
                consumableSlice.data(),
                consumableSlice.length());
        pOutputSampleBuffer += consumableSlice.length();
    }
    writableFrameRange.shrinkFront(consumableFrameRange.length());
    m_firstFrameIndex += consumableFrameRange.length();
    // Either the resulting output buffer or the sample buffer is
    // empty after returning from this function! Remaining data in
    // the sample buffer is kept until the next read operation.
    DEBUG_ASSERT(writableFrameRange.empty() || m_sampleBuffer.empty());

    // Return the remaining output buffer
    return WritableSampleFrames(
            writableFrameRange,
            SampleBuffer::WritableSlice(
                    pOutputSampleBuffer, writableFrameRange.length()));
}

} // namespace mixxx
