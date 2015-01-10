#include "sources/audiosource.h"

#include "sampleutil.h"

#include <vector>

namespace Mixxx {

/*static*/const AudioSource::sample_type AudioSource::kSampleValueZero =
        CSAMPLE_ZERO;
/*static*/const AudioSource::sample_type AudioSource::kSampleValuePeak =
        CSAMPLE_PEAK;

AudioSource::AudioSource()
        : m_channelCount(kChannelCountDefault), m_frameRate(kFrameRateDefault), m_frameCount(
                kFrameCountDefault), m_bitrate(kBitrateDefault) {
}

AudioSource::~AudioSource() {
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

void AudioSource::reset() {
    m_channelCount = kChannelCountDefault;
    m_frameRate = kFrameRateDefault;
    m_frameCount = kFrameCountDefault;
    m_bitrate = kBitrateDefault;
}

AudioSource::size_type AudioSource::readSampleFramesStereo(
        size_type numberOfFrames, sample_type* sampleBuffer) {
    switch (getChannelCount()) {
    case 1: // mono channel
    {
        const AudioSource::size_type readCount = readSampleFrames(
                numberOfFrames, sampleBuffer);
        SampleUtil::doubleMonoToDualMono(sampleBuffer, readCount);
        return readCount;
    }
    case 2: // stereo channel(s)
    {
        return readSampleFrames(numberOfFrames, sampleBuffer);
    }
    default: // multiple channels
    {
        typedef std::vector<sample_type> SampleBuffer;
        SampleBuffer tempBuffer(frames2samples(numberOfFrames));
        const AudioSource::size_type readCount = readSampleFrames(
                numberOfFrames, &tempBuffer[0]);
        SampleUtil::copyMultiToStereo(sampleBuffer, &tempBuffer[0], readCount,
                getChannelCount());
        return readCount;
    }
    }
}

}
