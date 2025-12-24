#include "sources/readaheadframebuffer.h"

#include "util/logger.h"
#include "util/sample.h"

// Override or set to `true` to enable verbose debug logging.
#if !defined(VERBOSE_DEBUG_LOG)
#define VERBOSE_DEBUG_LOG false
#endif

// Override or set to `true` to break with a debug assertion
// if an overlap or gap in the audio stream has been detected.
#if !defined(DEBUG_ASSERT_ON_DISCONTINUITIES)
#define DEBUG_ASSERT_ON_DISCONTINUITIES false
#endif

namespace mixxx {

namespace {

const Logger kLogger("ReadAheadFrameBuffer");

inline FrameCount validateCapacity(
        FrameCount capacity) {
    VERIFY_OR_DEBUG_ASSERT(capacity >= ReadAheadFrameBuffer::kEmptyCapacity) {
        return ReadAheadFrameBuffer::kEmptyCapacity;
    }
    return capacity;
}

} // anonymous namespace

ReadAheadFrameBuffer::ReadAheadFrameBuffer()
        : ReadAheadFrameBuffer(audio::SignalInfo(), kEmptyCapacity) {
}

ReadAheadFrameBuffer::ReadAheadFrameBuffer(
        const audio::SignalInfo& signalInfo,
        FrameCount initialCapacity)
        : m_signalInfo(signalInfo),
          m_sampleBuffer(
                  signalInfo.isValid()
                          ? m_signalInfo.frames2samples(
                                    validateCapacity(initialCapacity))
                          : kEmptyCapacity),
          m_readIndex(kUnknownFrameIndex) {
}

void ReadAheadFrameBuffer::adjustCapacityBeforeBuffering(
        FrameCount frameCount) {
    DEBUG_ASSERT(frameCount >= 0);
    const auto requiredCapacity = bufferedLength() + frameCount;
    if (capacity() < requiredCapacity) {
        // Increasing the pre-allocated capacity of the sample buffer should
        // never happen. The required capacity should always be known and
        // allocated in advance.
        kLogger.warning()
                << "Increasing capacity by reallocation:"
                << capacity()
                << "->"
                << requiredCapacity;
        m_sampleBuffer.adjustCapacity(
                m_signalInfo.frames2samples(requiredCapacity));
        DEBUG_ASSERT(
                m_signalInfo.frames2samples(frameCount) <= m_sampleBuffer.writableLength());
    }
}

bool ReadAheadFrameBuffer::tryContinueReadingFrom(
        FrameIndex readIndex) {
    if (!isReady()) {
        return false;
    }
    const auto clampedReadIndex = bufferedRange().clampIndex(readIndex);
    if (clampedReadIndex != readIndex) {
        return false;
    }
    DEBUG_ASSERT(clampedReadIndex >= m_readIndex);
    discardFirstBufferedFrames(clampedReadIndex - m_readIndex);
    return true;
}

void ReadAheadFrameBuffer::reset(
        FrameIndex currentIndex) {
    m_readIndex = currentIndex;
    m_sampleBuffer.clear();
    DEBUG_ASSERT(isValid());
}

FrameCount ReadAheadFrameBuffer::discardFirstBufferedFrames(
        FrameCount frameCount) {
    DEBUG_ASSERT(isReady());
    DEBUG_ASSERT(frameCount >= 0);
    DEBUG_ASSERT(frameCount <= bufferedLength());
    const auto shrinkCount = m_signalInfo.samples2frames(
            m_sampleBuffer.shrinkForReading(
                                  m_signalInfo.frames2samples(frameCount))
                    .length());
    m_readIndex += shrinkCount;
    return shrinkCount;
}

FrameCount ReadAheadFrameBuffer::discardLastBufferedFrames(
        FrameCount frameCount) {
    DEBUG_ASSERT(isReady());
    DEBUG_ASSERT(frameCount >= 0);
    DEBUG_ASSERT(frameCount <= bufferedLength());
    return m_sampleBuffer.shrinkAfterWriting(
            m_signalInfo.frames2samples(frameCount));
}

bool ReadAheadFrameBuffer::fillBuffer(
        const ReadableSampleFrames& inputBuffer) {
    DEBUG_ASSERT(isValid());
    const auto inputRange = inputBuffer.frameIndexRange();
    VERIFY_OR_DEBUG_ASSERT(inputRange.orientation() != IndexRange::Orientation::Backward) {
        return false;
    }
    const CSAMPLE* pInputSamples = inputBuffer.readableData();
    VERIFY_OR_DEBUG_ASSERT(pInputSamples) {
        return false;
    }

    // Overlapping input data has already been handled
    DEBUG_ASSERT(!isReady() || writeIndex() <= inputRange.start());
    if (!isReady()) {
        DEBUG_ASSERT(isEmpty());
        reset(inputRange.start());
    }
    DEBUG_ASSERT(isReady());

    // Detect and handle discontinuities: Gap
    if (writeIndex() < inputRange.start()) {
        const auto gapRange = IndexRange::between(writeIndex(), inputRange.start());
        DEBUG_ASSERT(gapRange.orientation() == IndexRange::Orientation::Forward);
        kLogger.warning()
                << "Missing range"
                << gapRange
                << "between internal buffer"
                << bufferedRange()
                << "and input buffer"
                << inputRange;
#if DEBUG_ASSERT_ON_DISCONTINUITIES
        DEBUG_ASSERT(!"Unexpected gap");
#endif
        const SINT clearFrameCount = gapRange.length();
        adjustCapacityBeforeBuffering(clearFrameCount);
        const SINT clearSampleCount =
                m_signalInfo.frames2samples(clearFrameCount);
        const SampleBuffer::WritableSlice writableSamples(
                m_sampleBuffer.growForWriting(clearSampleCount));
        DEBUG_ASSERT(writableSamples.length() == clearSampleCount);
        SampleUtil::clear(
                writableSamples.data(),
                clearSampleCount);
    }

    DEBUG_ASSERT(writeIndex() == inputRange.start());
    if (inputRange.empty()) {
        return true;
    }
    // Consume the readable sample data by copying it into the internal buffer
#if VERBOSE_DEBUG_LOG
    kLogger.debug()
            << "Buffering sample frames"
            << inputRange;
#endif
    adjustCapacityBeforeBuffering(inputRange.length());
    const auto copySampleCount =
            m_signalInfo.frames2samples(inputRange.length());
    const SampleBuffer::WritableSlice writableSamples(
            m_sampleBuffer.growForWriting(copySampleCount));
    DEBUG_ASSERT(writableSamples.length() == copySampleCount);
    SampleUtil::copy(
            writableSamples.data(),
            inputBuffer.readableData(),
            copySampleCount);
    pInputSamples += copySampleCount;
    return true;
}

WritableSampleFrames ReadAheadFrameBuffer::drainBuffer(
        const WritableSampleFrames& outputBuffer) {
    DEBUG_ASSERT(isValid());
    auto outputRange = outputBuffer.frameIndexRange();
#if VERBOSE_DEBUG_LOG
    kLogger.debug()
            << "drainBuffer:"
            << "bufferedRange()"
            << bufferedRange()
            << "outputRange"
            << outputRange;
#endif
    if (isEmpty() || outputBuffer.writableSlice().empty()) {
        return outputBuffer;
    }
    DEBUG_ASSERT(isReady());
    if (outputRange.start() < m_readIndex) {
        // Buffered data starts beyond the requested range
        return outputBuffer;
    }
    const auto consumableRange = intersect2(bufferedRange(), outputRange);
    DEBUG_ASSERT(!consumableRange || consumableRange->isSubrangeOf(outputRange));
    if (!consumableRange || consumableRange->empty()) {
        // No overlap between buffer and requested data
        return outputBuffer;
    }
    DEBUG_ASSERT(consumableRange->start() == outputRange.start());

    // Drop and skip any buffered samples preceding the requested range
#if VERBOSE_DEBUG_LOG
    if (m_readIndex < consumableRange->start()) {
        kLogger.debug()
                << "Discarding buffered frames"
                << IndexRange::between(m_readIndex, consumableRange->start());
    }
#endif
    discardFirstBufferedFrames(consumableRange->start() - m_readIndex);

    // Consume buffered samples
#if VERBOSE_DEBUG_LOG
    kLogger.debug()
            << "Consuming buffered samples" << *consumableRange
            << "outputRange" << outputRange
            << "readIndex()" << readIndex()
            << "bufferedRange()" << bufferedRange();
#endif
    DEBUG_ASSERT(m_readIndex == consumableRange->start());
    const SampleBuffer::ReadableSlice consumableSamples =
            m_sampleBuffer.shrinkForReading(
                    m_signalInfo.frames2samples(consumableRange->length()));
    DEBUG_ASSERT(consumableSamples.length() ==
            m_signalInfo.frames2samples(consumableRange->length()));
    CSAMPLE* pOutputSamples = outputBuffer.writableData();
    if (pOutputSamples) {
        SampleUtil::copy(
                pOutputSamples,
                consumableSamples.data(),
                consumableSamples.length());
        pOutputSamples += consumableSamples.length();
    }
    outputRange.shrinkFront(consumableRange->length());
    m_readIndex += consumableRange->length();

    // Either the resulting output buffer or the internal sample buffer
    // is empty after returning from this function!
    DEBUG_ASSERT(outputRange.empty() || isEmpty());

    // Return the remaining output buffer
    return WritableSampleFrames(
            outputRange,
            SampleBuffer::WritableSlice(
                    pOutputSamples,
                    m_signalInfo.frames2samples(outputRange.length())));
}

WritableSampleFrames ReadAheadFrameBuffer::consumeAndFillBuffer(
        const ReadableSampleFrames& inputBuffer,
        const WritableSampleFrames& outputBuffer,
        FrameCount minOutputIndex) {
    auto inputRange = inputBuffer.frameIndexRange();
    auto outputRange = outputBuffer.frameIndexRange();
#if VERBOSE_DEBUG_LOG
    kLogger.debug()
            << "consumeAndFillBuffer:"
            << "bufferedRange()"
            << bufferedRange()
            << "inputRange"
            << inputRange
            << "outputRange"
            << outputRange
            << "minOutputIndex"
            << minOutputIndex;
#endif

    const auto* pInputSampleData = inputBuffer.readableData();
    // input sample data is mandatory
    DEBUG_ASSERT(pInputSampleData);
    DEBUG_ASSERT(inputRange.orientation() != IndexRange::Orientation::Backward);
    DEBUG_ASSERT(isEmpty() || writeIndex() == inputRange.start());

    CSAMPLE* pOutputSampleData = outputBuffer.writableData();
    DEBUG_ASSERT(pOutputSampleData);
    DEBUG_ASSERT(outputRange.orientation() != IndexRange::Orientation::Backward);
    DEBUG_ASSERT(isEmpty() || outputRange.empty());
    DEBUG_ASSERT(minOutputIndex <= outputRange.start());

    // Detect and handle unexpected discontinuities: Overlap
    if (inputRange.start() < outputRange.start()) {
        const auto overlapRange = IndexRange::between(
                std::max(inputRange.start(), minOutputIndex),
                outputRange.start());
        DEBUG_ASSERT(
                overlapRange.orientation() !=
                IndexRange::Orientation::Backward);
        // Overlaps should never occur, but we cannot be sure
        if (overlapRange.orientation() ==
                IndexRange::Orientation::Forward) {
            kLogger.warning()
                    << "Overlapping range"
                    << overlapRange
                    << "in output buffer"
                    << outputRange
                    << "with input buffer"
                    << inputRange;
#if DEBUG_ASSERT_ON_DISCONTINUITIES
            DEBUG_ASSERT(!"Unexpected overlap");
#endif
            const SINT overlapingFrames = overlapRange.length();
            pOutputSampleData -= m_signalInfo.frames2samples(overlapingFrames);
            outputRange.growFront(overlapingFrames);
        }
    }
    if (!isEmpty() && inputRange.start() < writeIndex()) {
        const auto overlapRange = IndexRange::between(
                std::max(inputRange.start(), readIndex()),
                writeIndex());
        DEBUG_ASSERT(
                overlapRange.orientation() !=
                IndexRange::Orientation::Backward);
        // Overlaps should never occur, but we cannot be sure
        if (overlapRange.orientation() ==
                IndexRange::Orientation::Forward) {
            kLogger.warning()
                    << "Overlapping range"
                    << overlapRange
                    << "in internal buffer"
                    << bufferedRange()
                    << "with input buffer"
                    << inputRange;
#if DEBUG_ASSERT_ON_DISCONTINUITIES
            DEBUG_ASSERT(!"Unexpected overlap");
#endif
            discardLastBufferedFrames(overlapRange.length());
        }
    }

    // Discard input data that precedes outputRange
    if (inputRange.start() < outputRange.start()) {
        const auto precedingRange =
                IndexRange::between(
                        inputRange.start(),
                        std::min(outputRange.start(), inputRange.end()));
#if VERBOSE_DEBUG_LOG
        kLogger.debug()
                << "Discarding input data"
                << precedingRange
                << "that precedes"
                << outputRange;
#endif
        DEBUG_ASSERT(precedingRange.orientation() != IndexRange::Orientation::Backward);
        const auto precedingFrameCount = precedingRange.length();
        pInputSampleData += m_signalInfo.frames2samples(precedingFrameCount);
        inputRange.shrinkFront(precedingFrameCount);
    }

    // Fill output buffer
    if (!inputRange.empty() && !outputRange.empty()) {
        DEBUG_ASSERT(outputRange.start() <= inputRange.start());

        // Detect and handle discontinuities: Gap
        if (outputRange.start() < inputRange.start()) {
            const auto gapRange =
                    IndexRange::between(
                            outputRange.start(),
                            std::min(inputRange.start(), outputRange.end()));
            DEBUG_ASSERT(
                    gapRange.orientation() !=
                    IndexRange::Orientation::Backward);
            if (gapRange.orientation() == IndexRange::Orientation::Forward) {
                kLogger.warning()
                        << "Missing range"
                        << gapRange
                        << "between output buffer"
                        << outputRange
                        << "and input buffer"
                        << inputRange;
#if DEBUG_ASSERT_ON_DISCONTINUITIES
                DEBUG_ASSERT(!"Unexpected gap");
#endif
                const SINT clearFrameCount = gapRange.length();
                const SINT clearSampleCount = m_signalInfo.frames2samples(clearFrameCount);
                SampleUtil::clear(pOutputSampleData, clearSampleCount);
                pOutputSampleData += clearSampleCount;
                outputRange.shrinkFront(clearFrameCount);
            }
        }

        // Copy available input data
        const auto copyableFrameRange =
                IndexRange::between(
                        outputRange.start(),
                        std::min(inputRange.end(), outputRange.end()));
        DEBUG_ASSERT(copyableFrameRange.orientation() != IndexRange::Orientation::Backward);
        if (copyableFrameRange.orientation() == IndexRange::Orientation::Forward) {
#if VERBOSE_DEBUG_LOG
            kLogger.debug()
                    << "Copying available input data"
                    << copyableFrameRange
                    << "into output buffer";
#endif
            DEBUG_ASSERT(outputRange.start() == inputRange.start());
            const auto copyFrameCount = copyableFrameRange.length();
            const auto copySampleCount = m_signalInfo.frames2samples(copyFrameCount);
            SampleUtil::copy(
                    pOutputSampleData,
                    pInputSampleData,
                    copySampleCount);
            pOutputSampleData += copySampleCount;
            pInputSampleData += copySampleCount;
            inputRange.shrinkFront(copyFrameCount);
            outputRange.shrinkFront(copyFrameCount);
        }
    }

    // Reset internal write position according to the input data
    if (isEmpty()) {
        reset(inputRange.start());
    }
    DEBUG_ASSERT(isReady());
    DEBUG_ASSERT(writeIndex() == inputRange.start());

    // Fill internal buffer
    if (!inputRange.empty()) {
        bool success = fillBuffer(
                ReadableSampleFrames(
                        inputRange,
                        SampleBuffer::ReadableSlice(
                                pInputSampleData,
                                m_signalInfo.frames2samples(inputRange.length()))));
        Q_UNUSED(success)
        DEBUG_ASSERT(success);
    }

    // Return remaining output buffer
    return WritableSampleFrames(
            outputRange,
            SampleBuffer::WritableSlice(
                    pOutputSampleData,
                    m_signalInfo.frames2samples(outputRange.length())));
}

} // namespace mixxx
