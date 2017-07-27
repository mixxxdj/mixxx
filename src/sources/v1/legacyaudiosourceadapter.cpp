#include "sources/v1/legacyaudiosourceadapter.h"

#include "util/logger.h"


namespace mixxx {

namespace {

const Logger kLogger("LegacyAudioSourceAdapter");

} // anonymous namespace

LegacyAudioSourceAdapter::LegacyAudioSourceAdapter(
        AudioSource* pOwner,
        LegacyAudioSource* pImpl)
    : m_pOwner(pOwner),
      m_pImpl(pImpl) {
}

IndexRange LegacyAudioSourceAdapter::readOrSkipSampleFrames(
        IndexRange frameIndexRange,
        SampleBuffer::WritableSlice* pOutputBuffer) {
    auto readableFrames =
            m_pOwner->adjustReadableFrameIndexRangeAndOutputBuffer(
                    frameIndexRange, pOutputBuffer);
    if (readableFrames.empty()) {
        return readableFrames;
    }

    const SINT seekFrameIndex = m_pImpl->seekSampleFrame(readableFrames.start());
    if (seekFrameIndex < readableFrames.start()) {
        const auto precedingFrames =
                IndexRange::between(seekFrameIndex, readableFrames.start());
        kLogger.info()
                << "Skipping preceding frames"
                << precedingFrames;
        if (precedingFrames.length() != m_pImpl->readSampleFrames(precedingFrames.length(), nullptr)) {
            kLogger.warning()
                    << "Failed to skip preceding frames"
                    << precedingFrames;
            return IndexRange();
        }
    }
    DEBUG_ASSERT(seekFrameIndex >= readableFrames.start());

    SINT outputSampleOffset = 0;
    if (seekFrameIndex > readableFrames.start()) {
        const auto unreadableFrames = readableFrames.cutFrontRange(seekFrameIndex - readableFrames.start());
        kLogger.warning()
                << "Dropping unreadable frames"
                << unreadableFrames;
        outputSampleOffset += unreadableFrames.length();
    }
    DEBUG_ASSERT(seekFrameIndex == readableFrames.start());

    // Read or skip data
    return IndexRange::forward(
            readableFrames.start(),
            m_pImpl->readSampleFrames(
                    readableFrames.length(),
                    pOutputBuffer ? pOutputBuffer->data(outputSampleOffset) : nullptr));
}

} // namespace mixxx
