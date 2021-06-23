#include "sources/v1/legacyaudiosourceadapter.h"

#include "sources/audiosource.h"

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

ReadableSampleFrames LegacyAudioSourceAdapter::readSampleFramesClamped(
        const WritableSampleFrames& originalWritableSampleFrames) {
    const SINT firstFrameIndex = originalWritableSampleFrames.frameIndexRange().start();

    const SINT seekFrameIndex = m_pImpl->seekSampleFrame(firstFrameIndex);
    if (seekFrameIndex < firstFrameIndex) {
        const auto precedingFrames =
                IndexRange::between(seekFrameIndex, firstFrameIndex);
        kLogger.info()
                << "Skipping preceding frames"
                << precedingFrames;
        if (precedingFrames.length() != m_pImpl->readSampleFrames(precedingFrames.length(), nullptr)) {
            kLogger.warning()
                    << "Failed to skip preceding frames"
                    << precedingFrames;
            return ReadableSampleFrames();
        }
    }
    DEBUG_ASSERT(seekFrameIndex >= firstFrameIndex);

    WritableSampleFrames writableSampleFrames = originalWritableSampleFrames;
    if (seekFrameIndex > firstFrameIndex) {
        const SINT unreadableFrameOffset = seekFrameIndex - firstFrameIndex;
        kLogger.warning()
                << "Dropping"
                << unreadableFrameOffset
                << "unreadable frames";
        if (writableSampleFrames.frameIndexRange().containsIndex(seekFrameIndex)) {
            const auto remainingFrameIndexRange =
                    IndexRange::between(seekFrameIndex, writableSampleFrames.frameIndexRange().end());
            if (writableSampleFrames.writableData()) {
                writableSampleFrames = WritableSampleFrames(
                        remainingFrameIndexRange,
                        SampleBuffer::WritableSlice(
                                writableSampleFrames.writableData(m_pOwner->getSignalInfo().frames2samples(unreadableFrameOffset)),
                                m_pOwner->getSignalInfo().frames2samples(remainingFrameIndexRange.length())));
            } else {
                writableSampleFrames = WritableSampleFrames(remainingFrameIndexRange);
            }
        } else {
            writableSampleFrames = WritableSampleFrames();
        }
    }
    // Read or skip data
    const SINT numFramesRead =
            m_pImpl->readSampleFrames(
                    writableSampleFrames.frameLength(),
                    writableSampleFrames.writableData());
    const auto resultFrameIndexRange =
            IndexRange::forward(writableSampleFrames.frameIndexRange().start(), numFramesRead);
    return ReadableSampleFrames(
            resultFrameIndexRange,
            SampleBuffer::ReadableSlice(
                    writableSampleFrames.writableData(),
                    m_pOwner->getSignalInfo().frames2samples(resultFrameIndexRange.length())));
}

} // namespace mixxx
