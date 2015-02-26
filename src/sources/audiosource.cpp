#include "sources/audiosource.h"

#include "samplebuffer.h"
#include "sampleutil.h"

#include <vector>

namespace Mixxx {

/*static*/const CSAMPLE AudioSource::kSampleValueZero =
        CSAMPLE_ZERO;
/*static*/const CSAMPLE AudioSource::kSampleValuePeak =
        CSAMPLE_PEAK;

AudioSourcePointer AudioSource::onCreate(AudioSource* pNewAudioSource) {
    AudioSourcePointer pAudioSource(pNewAudioSource); // take ownership
    if (OK == pAudioSource->postConstruct()) {
        // success
        return pAudioSource;
    } else {
        // failure
        return AudioSourcePointer();
    }
}

AudioSource::AudioSource(QUrl url)
        : UrlResource(url),
          m_channelCount(kChannelCountDefault),
          m_frameRate(kFrameRateDefault),
          m_frameCount(kFrameCountDefault),
          m_bitrate(kBitrateDefault) {
}

void AudioSource::setChannelCount(SINT channelCount) {
    m_channelCount = channelCount;
}
void AudioSource::setFrameRate(SINT frameRate) {
    m_frameRate = frameRate;
}
void AudioSource::setFrameCount(SINT frameCount) {
    m_frameCount = frameCount;
}

SINT AudioSource::getSampleBufferSize(
        SINT numberOfFrames,
        bool readStereoSamples) const {
    if (readStereoSamples) {
        return numberOfFrames * 2;
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
