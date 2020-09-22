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
        ReadableSampleFrames inputSampleFrames,
        BufferingMode bufferingMode) {
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
        VERIFY_OR_DEBUG_ASSERT(writeIndex() <= inputRange.start()) {
            kLogger.warning()
                    << "Cannot buffer sample data from"
                    << inputRange
                    << "starting before"
                    << writeIndex();
            return inputSampleFrames;
        }
    } else {
        DEBUG_ASSERT(isEmpty());
        reset(inputRange.start());
    }
    DEBUG_ASSERT(isReady());
    if (inputRange.start() > writeIndex()) {
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
                    << IndexRange::between(writeIndex(), inputRange.start());
#endif
            const auto clearFrameCount =
                    inputRange.start() - writeIndex();
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
    DEBUG_ASSERT(writeIndex() == inputRange.start());
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

WritableSampleFrames ReadAheadFrameBuffer::drain(
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

WritableSampleFrames ReadAheadFrameBuffer::recharge(
        ReadableSampleFrames inputSampleFrames,
        WritableSampleFrames outputSampleFrames,
        FrameCount minOutputIndex,
        BufferingMode bufferingMode) {
    auto inputRange = inputSampleFrames.frameIndexRange();
    auto outputRange = outputSampleFrames.frameIndexRange();
#if VERBOSE_DEBUG_LOG
    kLogger.debug()
            << "Recharging:"
            << "bufferedRange()"
            << bufferedRange()
            << "inputRange"
            << inputRange
            << "outputRange"
            << outputRange
            << "minOutputIndex"
            << minOutputIndex;
#endif

    const auto* pInputSampleData = inputSampleFrames.readableData();
    // input sample data is mandatory
    DEBUG_ASSERT(pInputSampleData);
    DEBUG_ASSERT(inputRange.orientation() != IndexRange::Orientation::Backward);
    DEBUG_ASSERT(isEmpty() || writeIndex() == inputRange.start());

    // output sample data is optional, i.e. input samples will be dropped
    // and not copied if no output buffer is provided
    auto* pOutputSampleData = outputSampleFrames.writableData();
    DEBUG_ASSERT(outputRange.orientation() != IndexRange::Orientation::Backward);
    DEBUG_ASSERT(isEmpty() || outputRange.empty());
    DEBUG_ASSERT(minOutputIndex <= outputRange.start());

    // Rewind if input and output ranges overlap
    if (inputRange.start() < outputRange.start()) {
        const auto rewindRange = IndexRange::between(
                math_max(inputRange.start(), minOutputIndex),
                outputRange.start());
        VERIFY_OR_DEBUG_ASSERT(
                rewindRange.orientation() ==
                IndexRange::Orientation::Empty) {
            kLogger.warning()
                    << "Discarding"
                    << rewindRange
                    << "overlapping with input buffer"
                    << inputRange
                    << "from output buffer"
                    << outputRange;
            if (pOutputSampleData) {
                pOutputSampleData -=
                        m_signalInfo.frames2samples(rewindRange.length());
            }
            outputRange.growFront(rewindRange.length());
        }
    }
    if (!isEmpty() && inputRange.start() < writeIndex()) {
        const auto rewindRange = IndexRange::between(
                math_max(inputRange.start(), readIndex()),
                writeIndex());
        VERIFY_OR_DEBUG_ASSERT(
                rewindRange.orientation() ==
                IndexRange::Orientation::Empty) {
            kLogger.warning()
                    << "Discarding"
                    << rewindRange
                    << "overlapping with input buffer"
                    << inputRange
                    << "from internal buffer"
                    << bufferedRange();
            discardLastBufferedFrames(
                    rewindRange.length());
        }
    }

    // Discard input data that precedes outputRange
    if (inputRange.start() < outputRange.start()) {
        const auto precedingRange =
                IndexRange::between(
                        inputRange.start(),
                        math_min(outputRange.start(), inputRange.end()));
#if VERBOSE_DEBUG_LOG
        kLogger.debug()
                << "Discarding input data"
                << precedingRange
                << "that precedes"
                << outputRange;
#endif
        DEBUG_ASSERT(precedingRange.orientation() == IndexRange::Orientation::Forward);
        const auto precedingFrameCount = precedingRange.length();
        pInputSampleData += m_signalInfo.frames2samples(precedingFrameCount);
        inputRange.shrinkFront(precedingFrameCount);
    }

    // Fill output buffer
    if (!inputRange.empty() && !outputRange.empty()) {
        DEBUG_ASSERT(outputRange.start() <= inputRange.start());

        // Substitute missing input data with silence
        if (outputRange.start() < inputRange.start()) {
            const auto missingFrameRange =
                    IndexRange::between(
                            outputRange.start(),
                            math_min(inputRange.start(), outputRange.end()));
#if VERBOSE_DEBUG_LOG
            kLogger.debug()
                    << "Substituting missing input data"
                    << missingFrameRange
                    << "with silence in output buffer";
#endif
            DEBUG_ASSERT(missingFrameRange.orientation() == IndexRange::Orientation::Forward);
            const auto clearFrameCount = missingFrameRange.length();
            const auto clearSampleCount = m_signalInfo.frames2samples(clearFrameCount);
            if (pOutputSampleData) {
                SampleUtil::clear(
                        pOutputSampleData,
                        clearSampleCount);
                pOutputSampleData += clearSampleCount;
            }
            outputRange.shrinkFront(clearFrameCount);
        }

        // Copy available input data
        const auto copyableFrameRange =
                IndexRange::between(
                        outputRange.start(),
                        math_min(inputRange.end(), outputRange.end()));
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
            if (pOutputSampleData) {
                SampleUtil::copy(
                        pOutputSampleData,
                        pInputSampleData,
                        copySampleCount);
                pOutputSampleData += copySampleCount;
            }
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
        inputSampleFrames = bufferSampleData(
                ReadableSampleFrames(
                        inputRange,
                        SampleBuffer::ReadableSlice(
                                pInputSampleData,
                                m_signalInfo.frames2samples(inputRange.length()))),
                bufferingMode);
        Q_UNUSED(inputSampleFrames)
        DEBUG_ASSERT(inputSampleFrames.frameIndexRange().empty());
    }

    // Return remaining output buffer
    return WritableSampleFrames(
            outputRange,
            SampleBuffer::WritableSlice(
                    pOutputSampleData,
                    m_signalInfo.frames2samples(outputRange.length())));
}

} // namespace mixxx
