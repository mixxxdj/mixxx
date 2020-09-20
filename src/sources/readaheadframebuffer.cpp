#include "sources/readaheadframebuffer.h"

#include "util/logger.h"
#include "util/sample.h"

#if !defined(VERBOSE_DEBUG_LOG)
#define VERBOSE_DEBUG_LOG false
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

void ReadAheadFrameBuffer::beforeBuffering(
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
    DEBUG_ASSERT(isValid());
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

ReadableSampleFrames ReadAheadFrameBuffer::bufferSampleData(
        BufferingMode bufferingMode,
        ReadableSampleFrames inputSampleFrames) {
    DEBUG_ASSERT(isValid());
    auto inputRange = inputSampleFrames.frameIndexRange();
    VERIFY_OR_DEBUG_ASSERT(inputRange.orientation() != IndexRange::Orientation::Backward) {
        return inputSampleFrames;
    }
    const CSAMPLE* pInputSamples = inputSampleFrames.readableData();
    VERIFY_OR_DEBUG_ASSERT(pInputSamples) {
        return inputSampleFrames;
    }
    if (isReady()) {
        VERIFY_OR_DEBUG_ASSERT(bufferedRange().end() <= inputRange.start()) {
            kLogger.warning()
                    << "Cannot buffer sample data from"
                    << inputRange
                    << "starting before"
                    << bufferedRange().end();
            return inputSampleFrames;
        }
    } else {
        DEBUG_ASSERT(isEmpty());
        reset(inputRange.start());
    }
    DEBUG_ASSERT(isReady());
    if (inputRange.start() > bufferedRange().end()) {
        // Gap between current write position and the start of
        // the input sample data
        switch (bufferingMode) {
        case BufferingMode::SkipGapAndReset:
            reset(inputRange.start());
            break;
        case BufferingMode::FillGapWithSilence: {
#if VERBOSE_DEBUG_LOG
            kLogger.debug()
                    << "Filling gap with silence"
                    << IndexRange::between(writeIndex, inputRange.start());
#endif
            const auto clearFrameCount =
                    inputRange.start() - bufferedRange().end();
            beforeBuffering(clearFrameCount);
            const auto clearSampleCount =
                    m_signalInfo.frames2samples(clearFrameCount);
            const SampleBuffer::WritableSlice writableSamples(
                    m_sampleBuffer.growForWriting(clearSampleCount));
            DEBUG_ASSERT(writableSamples.length() == clearSampleCount);
            SampleUtil::clear(
                    writableSamples.data(),
                    clearSampleCount);
        } break;
        default:
            DEBUG_ASSERT(!"unexpected BufferingMode");
        }
    }
    DEBUG_ASSERT(bufferedRange().end() == inputRange.start());
    if (inputRange.empty()) {
        return inputSampleFrames;
    }
    // Consume the readable sample data by copying it into the internal buffer
#if VERBOSE_DEBUG_LOG
    kLogger.debug()
            << "Buffering sample frames"
            << inputRange;
#endif
    beforeBuffering(inputRange.length());
    const auto copySampleCount =
            m_signalInfo.frames2samples(inputRange.length());
    const SampleBuffer::WritableSlice writableSamples(
            m_sampleBuffer.growForWriting(copySampleCount));
    DEBUG_ASSERT(writableSamples.length() == copySampleCount);
    SampleUtil::copy(
            writableSamples.data(),
            inputSampleFrames.readableData(),
            copySampleCount);
    pInputSamples += copySampleCount;
    inputRange.shrinkFront(inputRange.length());
    DEBUG_ASSERT(inputRange.empty());
    return ReadableSampleFrames(
            inputRange,
            SampleBuffer::ReadableSlice(
                    pInputSamples,
                    m_signalInfo.frames2samples(inputRange.length())));
}

WritableSampleFrames ReadAheadFrameBuffer::consumeBufferedSampleData(
        WritableSampleFrames outputSampleFrames) {
    DEBUG_ASSERT(isValid());
    if (isEmpty() || outputSampleFrames.writableSlice().empty()) {
        return outputSampleFrames;
    }
    DEBUG_ASSERT(isReady());
    auto outputRange = outputSampleFrames.frameIndexRange();
    if (outputRange.start() < m_readIndex) {
        // Buffered data starts beyond the requested range
        return outputSampleFrames;
    }
    const auto consumableRange = intersect(bufferedRange(), outputRange);
    DEBUG_ASSERT(consumableRange <= outputRange);
    if (consumableRange.empty()) {
        // No overlap between buffer and requested data
        return outputSampleFrames;
    }
    DEBUG_ASSERT(consumableRange.start() == outputRange.start());

    // Drop and skip any buffered samples preceding the requested range
#if VERBOSE_DEBUG_LOG
    if (m_readIndex < consumableRange.start()) {
        kLogger.debug()
                << "Discarding buffered frames"
                << IndexRange::between(m_readIndex, consumableRange.start());
    }
#endif
    discardFirstBufferedFrames(consumableRange.start() - m_readIndex);

    // Consume buffered samples
#if VERBOSE_DEBUG_LOG
    kLogger.debug()
            << "Consuming buffered samples" << consumableRange
            << "outputRange" << outputRange
            << "readIndex()" << readIndex()
            << "bufferedRange()" << bufferedRange();
#endif
    DEBUG_ASSERT(m_readIndex == consumableRange.start());
    const SampleBuffer::ReadableSlice consumableSamples =
            m_sampleBuffer.shrinkForReading(
                    m_signalInfo.frames2samples(consumableRange.length()));
    DEBUG_ASSERT(consumableSamples.length() ==
            m_signalInfo.frames2samples(consumableRange.length()));
    CSAMPLE* pOutputSamples = outputSampleFrames.writableData();
    if (pOutputSamples) {
        SampleUtil::copy(
                pOutputSamples,
                consumableSamples.data(),
                consumableSamples.length());
        pOutputSamples += consumableSamples.length();
    }
    outputRange.shrinkFront(consumableRange.length());
    m_readIndex += consumableRange.length();

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

} // namespace mixxx
