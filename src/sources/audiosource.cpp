#include "sources/audiosource.h"

#include "sampleutil.h"

namespace Mixxx {

AudioSource::AudioSource(const QUrl& url)
        : UrlResource(url),
          m_channelCount(kChannelCountDefault),
          m_frameRate(kFrameRateDefault),
          m_frameCount(kFrameCountDefault),
          m_bitrate(kBitrateDefault) {
}

void AudioSource::setChannelCount(SINT channelCount) {
    DEBUG_ASSERT(isValidChannelCount(channelCount));
    m_channelCount = channelCount;
}

void AudioSource::setFrameRate(SINT frameRate) {
    DEBUG_ASSERT(isValidFrameRate(frameRate));
    m_frameRate = frameRate;
}

void AudioSource::setFrameCount(SINT frameCount) {
    DEBUG_ASSERT(isValidFrameCount(frameCount));
    m_frameCount = frameCount;
}

void AudioSource::setBitrate(SINT bitrate) {
    DEBUG_ASSERT(isValidBitrate(bitrate));
    m_bitrate = bitrate;
}

SINT AudioSource::getSampleBufferSize(
        SINT numberOfFrames,
        bool readStereoSamples) const {
    if (readStereoSamples) {
        return numberOfFrames * kChannelCountStereo;
    } else {
        return frames2samples(numberOfFrames);
    }
}

SINT AudioSource::readSampleFramesStereo(
        SINT numberOfFrames,
        CSAMPLE* sampleBuffer,
        SINT sampleBufferSize) {
    DEBUG_ASSERT(getSampleBufferSize(numberOfFrames, true) <= sampleBufferSize);

    switch (getChannelCount()) {
        case 1: // mono channel
        {
            const SINT readFrameCount = readSampleFrames(
                    numberOfFrames, sampleBuffer);
            SampleUtil::doubleMonoToDualMono(sampleBuffer, readFrameCount);
            return readFrameCount;
        }
        case 2: // stereo channel(s)
        {
            return readSampleFrames(numberOfFrames, sampleBuffer);
        }
        default: // multiple (3 or more) channels
        {
            const SINT numberOfSamplesToRead = frames2samples(numberOfFrames);
            if (numberOfSamplesToRead <= sampleBufferSize) {
                // efficient in-place transformation
                const SINT readFrameCount = readSampleFrames(
                        numberOfFrames, sampleBuffer);
                SampleUtil::copyMultiToStereo(sampleBuffer, sampleBuffer,
                        readFrameCount, getChannelCount());
                return readFrameCount;
            } else {
                // inefficient transformation through a temporary buffer
                qDebug() << "Performance warning:"
                        << "Allocating a temporary buffer of size"
                        << numberOfSamplesToRead << "for reading stereo samples."
                        << "The size of the provided sample buffer is"
                        << sampleBufferSize;
                SampleBuffer tempBuffer(numberOfSamplesToRead);
                const SINT readFrameCount = readSampleFrames(
                        numberOfFrames, tempBuffer.data());
                SampleUtil::copyMultiToStereo(sampleBuffer, tempBuffer.data(),
                        readFrameCount, getChannelCount());
                return readFrameCount;
            }
        }
    }
}

}
