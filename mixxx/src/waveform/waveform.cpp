#include "waveform/waveform.h"

#include <cstring>

#include <QDebug>

Waveform::Waveform() :
    m_size(0),
    m_textureStride(1024),
    m_completion(0),
    m_audioSamplesPerVisualSample(0),
    m_visualSampleRate(0),
    m_audioVisualRatio(0) {
}

Waveform::~Waveform() {
}

void Waveform::computeBestVisualSampleRate( int audioSampleRate, double desiredVisualSampleRate) {

    m_audioSamplesPerVisualSample = (int)((double)audioSampleRate / desiredVisualSampleRate);
    const double actualVisualSamplingRate = (double)audioSampleRate / (double)(m_audioSamplesPerVisualSample);

    m_visualSampleRate = actualVisualSamplingRate;
    m_audioVisualRatio = (double)audioSampleRate / (double)m_visualSampleRate;
}

void Waveform::allocate( int audioSamples) {
    int numberOfVisualSamples = audioSamples / m_audioSamplesPerVisualSample + 1;
    numberOfVisualSamples += numberOfVisualSamples%2;
    assign(numberOfVisualSamples,0);
    setCompletion(0);
}

void Waveform::resize(int size) {
    m_size = size;
    int textureSize = computeTextureSize(size);
    m_data.resize(textureSize);
}

void Waveform::assign(int size, int value) {
    m_size = size;
    int textureSize = computeTextureSize(size);
    m_data.assign(textureSize,value);
}

int Waveform::computeTextureSize(int size) {

    //find the best match
    //NOTE vRince : I know 'what about actual coding ? ...'

    if( size <= 256*256) { //~1min @441Hz stereo
        m_textureStride = 256;
        return 256*256;
    } else if( size <= 512*512) { //~9min @441Hz stereo
        m_textureStride = 512;
        return 512*512;
    } else if( size <= 1024*1024) { //~19min @441Hz stereo
        m_textureStride = 1024;
        return 1024*1024;
    } else if( size <= 2048*2048) { //~79min @441Hz stereo
        m_textureStride = 2048;
        return 2048*2048;
    } else {  //~317min @441Hz stereo

        if( size > 4096*4096)
            qDebug() << "Waveform::computeTextureSize - this look like a really big song ...";

        m_textureStride = 4096;
        return 4096*4096;
    }
}

void Waveform::dump() const {
    qDebug() << "Waveform" << this
             << "size("+QString::number(m_size)+")"
             << "textureStride("+QString::number(m_textureStride)+")"
             << "completion("+QString::number(m_completion)+")"
             << "audioSamplesPerVisualSample("+QString::number(m_audioSamplesPerVisualSample)+")"
             << "visualSampleRate("+QString::number(m_visualSampleRate)+")"
             << "audioVisualRatio("+QString::number(m_audioVisualRatio)+")";
}
