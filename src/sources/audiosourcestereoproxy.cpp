#include "sources/audiosourcestereoproxy.h"

#include "util/sample.h"
#include "util/logger.h"


namespace mixxx {

namespace {

const Logger kLogger("AudioSourceStereoProxy");

} // anonymous namespace

AudioSourceStereoProxy::AudioSourceStereoProxy(
        AudioSourcePointer pAudioSource,
        SampleBuffer::WritableSlice tempSampleBuffer)
    : AudioSource(*pAudioSource),
      m_pAudioSource(std::move(pAudioSource)),
      m_tempSampleBuffer(std::move(tempSampleBuffer)) {
    setChannelCount(ChannelCount::stereo());
}

namespace {

inline
bool isDisjunct(
        const SampleBuffer::WritableSlice& slice1,
        const SampleBuffer::WritableSlice& slice2) {
    if (slice1.data() == slice2.data()) {
        return false;
    }
    if ((slice1.size() == 0) || (slice2.size() == 0)) {
        return true;
    }
    if (slice1.data() < slice2.data()) {
        return slice1.data(slice1.size()) <= slice2.data();
    } else {
        return slice2.data(slice2.size()) <= slice1.data();
    }
}

}

IndexRange AudioSourceStereoProxy::readOrSkipSampleFrames(
        IndexRange frameIndexRange,
        SampleBuffer::WritableSlice* pOutputBuffer) {
    if (!pOutputBuffer) {
        return m_pAudioSource->skipSampleFrames(frameIndexRange);
    }
    if (m_pAudioSource->channelCount() == channelCount()) {
        return m_pAudioSource->readOrSkipSampleFrames(frameIndexRange, pOutputBuffer);
    }

    // Check location and capacity of temporary buffer
    VERIFY_OR_DEBUG_ASSERT(isDisjunct(m_tempSampleBuffer, *pOutputBuffer)) {
        kLogger.warning()
                << "Overlap between output and temporary sample buffer detected";
        return IndexRange();
    }
    {
        const SINT numberOfSamplesToRead = m_pAudioSource->frames2samples(frameIndexRange.length());
        VERIFY_OR_DEBUG_ASSERT(m_tempSampleBuffer.size() >= numberOfSamplesToRead) {
            kLogger.warning()
                    << "Insufficient temporary sample buffer capacity"
                    << m_tempSampleBuffer.size()
                    << "<"
                    << numberOfSamplesToRead
                    << "for reading frames"
                    << frameIndexRange;
            return IndexRange();
        }
    }

    const IndexRange resultFrameIndexRange =
            m_pAudioSource->readSampleFrames(
                    frameIndexRange,
                    m_tempSampleBuffer);
    DEBUG_ASSERT(resultFrameIndexRange <= frameIndexRange);
    if (!resultFrameIndexRange.empty()) {
        DEBUG_ASSERT(resultFrameIndexRange.start() >= frameIndexRange.start());
        const SINT frameOffset =
                resultFrameIndexRange.start() - frameIndexRange.start();
        const auto pDstSamples =
                pOutputBuffer->data(frames2samples(frameOffset));
        const auto pSrcSamples =
                m_tempSampleBuffer.data(m_pAudioSource->frames2samples(frameOffset));
        if (m_pAudioSource->channelCount().isMono()) {
            SampleUtil::copyMonoToDualMono(
                    pDstSamples,
                    pSrcSamples,
                    resultFrameIndexRange.length());
        } else {
            SampleUtil::copyMultiToStereo(
                    pDstSamples,
                    pSrcSamples,
                    resultFrameIndexRange.length(),
                    m_pAudioSource->channelCount());
        }
    }
    return resultFrameIndexRange;
}

}
