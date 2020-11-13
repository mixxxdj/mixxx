#include "sources/audiosourcestereoproxy.h"

#include "util/logger.h"
#include "util/sample.h"

namespace mixxx {

namespace {

const Logger kLogger("AudioSourceStereoProxy");

constexpr audio::ChannelCount kChannelCount = audio::ChannelCount(2);

audio::SignalInfo proxySignalInfo(
        const audio::SignalInfo& signalInfo) {
    DEBUG_ASSERT(signalInfo.isValid());
    return audio::SignalInfo(
            kChannelCount,
            signalInfo.getSampleRate(),
            signalInfo.getSampleLayout());
}

} // anonymous namespace

AudioSourceStereoProxy::AudioSourceStereoProxy(
        AudioSourcePointer pAudioSource,
        SINT maxReadableFrames)
        : AudioSourceProxy(
                std::move(pAudioSource),
                proxySignalInfo(pAudioSource->getSignalInfo())),
          m_tempSampleBuffer(
                  (m_pAudioSource->getSignalInfo().getChannelCount() != kChannelCount) ?
                  m_pAudioSource->getSignalInfo().frames2samples(maxReadableFrames) :
                  0),
          m_tempWritableSlice(m_tempSampleBuffer) {
}

AudioSourceStereoProxy::AudioSourceStereoProxy(
        AudioSourcePointer pAudioSource,
        SampleBuffer::WritableSlice tempWritableSlice)
        : AudioSourceProxy(
                std::move(pAudioSource),
                proxySignalInfo(pAudioSource->getSignalInfo())),
          m_tempWritableSlice(std::move(tempWritableSlice)) {
}

namespace {

inline bool isDisjunct(
        const SampleBuffer::WritableSlice& slice1,
        const SampleBuffer::WritableSlice& slice2) {
    if (slice1.data() == slice2.data()) {
        return false;
    }
    if ((slice1.length() == 0) || (slice2.length() == 0)) {
        return true;
    }
    if (slice1.data() < slice2.data()) {
        return slice1.data(slice1.length()) <= slice2.data();
    } else {
        return slice2.data(slice2.length()) <= slice1.data();
    }
}

} // namespace

ReadableSampleFrames AudioSourceStereoProxy::readSampleFramesClamped(
        WritableSampleFrames sampleFrames) {
    if (m_pAudioSource->getSignalInfo().getChannelCount() == kChannelCount) {
        return readSampleFramesClampedOn(*m_pAudioSource, sampleFrames);
    }

    // Check location and capacity of temporary buffer
    VERIFY_OR_DEBUG_ASSERT(isDisjunct(
            m_tempWritableSlice,
            SampleBuffer::WritableSlice(sampleFrames.writableSlice()))) {
        kLogger.warning()
                << "Overlap between output and temporary sample buffer detected";
        return ReadableSampleFrames();
    }
    {
        const SINT numberOfSamplesToRead =
                m_pAudioSource->getSignalInfo().frames2samples(
                        sampleFrames.frameLength());
        VERIFY_OR_DEBUG_ASSERT(m_tempWritableSlice.length() >= numberOfSamplesToRead) {
            kLogger.warning()
                    << "Insufficient temporary sample buffer capacity"
                    << m_tempWritableSlice.length()
                    << "<"
                    << numberOfSamplesToRead
                    << "for reading frames"
                    << sampleFrames.frameIndexRange();
            return ReadableSampleFrames();
        }
    }

    const auto readableSampleFrames =
            readSampleFramesClampedOn(
                    *m_pAudioSource,
                    WritableSampleFrames(
                            sampleFrames.frameIndexRange(),
                            m_tempWritableSlice));
    if (readableSampleFrames.frameIndexRange().empty()) {
        return readableSampleFrames;
    }
    DEBUG_ASSERT(
            readableSampleFrames.frameIndexRange().isSubrangeOf(sampleFrames.frameIndexRange()));
    DEBUG_ASSERT(
            readableSampleFrames.frameIndexRange().start() >=
            sampleFrames.frameIndexRange().start());
    const SINT frameOffset =
            readableSampleFrames.frameIndexRange().start() -
            sampleFrames.frameIndexRange().start();
    SampleBuffer::WritableSlice writableSlice(
            sampleFrames.writableData(getSignalInfo().frames2samples(frameOffset)),
            getSignalInfo().frames2samples(readableSampleFrames.frameLength()));
    if (m_pAudioSource->getSignalInfo().getChannelCount() == 1) {
        SampleUtil::copyMonoToDualMono(
                writableSlice.data(),
                readableSampleFrames.readableData(),
                readableSampleFrames.frameLength());
    } else {
        SampleUtil::copyMultiToStereo(
                writableSlice.data(),
                readableSampleFrames.readableData(),
                readableSampleFrames.frameLength(),
                m_pAudioSource->getSignalInfo().getChannelCount());
    }
    return ReadableSampleFrames(
            readableSampleFrames.frameIndexRange(),
            SampleBuffer::ReadableSlice(
                    writableSlice.data(),
                    writableSlice.length()));
}

} // namespace mixxx
