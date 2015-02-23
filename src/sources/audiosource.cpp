#include "sources/audiosource.h"

#include "samplebuffer.h"
#include "sampleutil.h"

#include <vector>

namespace Mixxx {

/*static*/const AudioSource::sample_type AudioSource::kSampleValueZero =
        CSAMPLE_ZERO;
/*static*/const AudioSource::sample_type AudioSource::kSampleValuePeak =
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

void AudioSource::setChannelCount(size_type channelCount) {
    m_channelCount = channelCount;
}
void AudioSource::setFrameRate(size_type frameRate) {
    m_frameRate = frameRate;
}
void AudioSource::setFrameCount(size_type frameCount) {
    m_frameCount = frameCount;
}

AudioSource::size_type AudioSource::getSampleBufferSize(
        size_type numberOfFrames,
        bool readStereoSamples) const {
    if (readStereoSamples) {
        return numberOfFrames * 2;
    } else {
        return frames2samples(numberOfFrames);
    }
}

AudioSource::size_type AudioSource::readSampleFramesStereo(
        size_type numberOfFrames,
        sample_type* sampleBuffer,
        size_type sampleBufferSize) {
    DEBUG_ASSERT(getSampleBufferSize(numberOfFrames, true) <= sampleBufferSize);

    switch (getChannelCount()) {
        case 1: // mono channel
        {
            const AudioSource::size_type readFrameCount = readSampleFrames(
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
            const size_type numberOfSamplesToRead = frames2samples(numberOfFrames);
            if (numberOfSamplesToRead <= sampleBufferSize) {
                // efficient in-place transformation
                const AudioSource::size_type readFrameCount = readSampleFrames(
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
                const AudioSource::size_type readFrameCount = readSampleFrames(
                        numberOfFrames, tempBuffer.data());
                SampleUtil::copyMultiToStereo(sampleBuffer, tempBuffer.data(),
                        readFrameCount, getChannelCount());
                return readFrameCount;
            }
        }
    }
}

}
