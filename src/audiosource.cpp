#include "audiosource.h"

#include "sampleutil.h"

#include <QDebug>

#include <vector>

namespace Mixxx {

/*static*/const AudioSource::sample_type AudioSource::kSampleValueZero =
        CSAMPLE_ZERO;
/*static*/const AudioSource::sample_type AudioSource::kSampleValuePeak =
        CSAMPLE_PEAK;

AudioSource::AudioSource()
        : m_channelCount(kChannelCountDefault), m_sampleRate(
                kSampleRateDefault), m_frameCount(kFrameCountDefault) {
}

AudioSource::~AudioSource() {
}

void AudioSource::setChannelCount(size_type channelCount) {
    m_channelCount = channelCount;
}

void AudioSource::setSampleRate(size_type sampleRate) {
    m_sampleRate = sampleRate;
}

AudioSource::size_type AudioSource::readStereoFrameSamplesInterleaved(
        size_type frameCount, sample_type* sampleBuffer) {
    switch (getChannelCount()) {
    case 1: // mono
    {
        AudioSource::size_type readCount = readFrameSamplesInterleaved(
                frameCount, sampleBuffer);
        SampleUtil::doubleMonoToDualMono(sampleBuffer, readCount);
        return readCount;
    }
    case 2: // stereo
    {
        return readFrameSamplesInterleaved(frameCount, sampleBuffer);
    }
    default: // multiple-channels
    {
        Q_ASSERT(2 < getChannelCount());
        typedef std::vector<sample_type> SampleBuffer;
        SampleBuffer tempBuffer(frames2samples(frameCount));
        AudioSource::size_type readCount = readFrameSamplesInterleaved(
                frameCount, &tempBuffer[0]);
        SampleUtil::copyMultiToStereo(sampleBuffer, &tempBuffer[0],
                readCount, getChannelCount());
        return readCount;
    }
    }
}

}
