#include "analyserwaveform.h"

#include <QVector>
#include <QtDebug>

#include <time.h>
#include <limits>

#include "trackinfoobject.h"
#include "waveform/waveform.h"

#include "engine/enginefilterbutterworth8.h"
#include "engine/enginefilteriir.h"

#include <QTime>

AnalyserWaveform::AnalyserWaveform() {
    m_ready = false;
    m_waveform = 0;
    m_filter[0] = 0;
    m_filter[1] = 0;
    m_filter[2] = 0;
    timer_ = new QTime();
}

AnalyserWaveform::~AnalyserWaveform()
{
    destroyFilters();
    delete timer_;
}

void AnalyserWaveform::initialise(TrackPointer tio, int sampleRate, int totalSamples) {
    timer_->start();
    m_waveform = tio->getWaveForm();

    if( !m_waveform || totalSamples == 0) {
        return;
    }

    destroyFilters();
    resetFilters(tio);

    const int samplesPerVisualSample = (int)((double)sampleRate / tio->getWaveForm()->getVisualSampleRate());
    const double actualVisualSamplingRate = samplesPerVisualSample;

    int numberOfVisualSamples = totalSamples / samplesPerVisualSample + 1;

    if(numberOfVisualSamples % 2 != 0)
        numberOfVisualSamples++;

    //qDebug() << "sampleRate" << sampleRate;
    //qDebug() << "samplesPerVisualSample" << samplesPerVisualSample;
    //qDebug() << "actualVisualSamplingRate" << actualVisualSamplingRate;
    //qDebug() << "numberOfVisualSamples" << numberOfVisualSamples;

    m_waveform->setAudioVisualRatio(samplesPerVisualSample);
    m_waveform->setVisualSampleRate(actualVisualSamplingRate);
    m_waveform->assign(numberOfVisualSamples,0);

    resetCurrentStride();

    m_strideLength = samplesPerVisualSample*2;
    m_timePerStride = samplesPerVisualSample;
    m_currentStride = 0;
    m_strideBufferPos = 0;

    m_convertionFactor = (float)std::numeric_limits<unsigned char>::max()/m_timePerStride;

    m_ready = true;
}

void AnalyserWaveform::resetFilters(TrackPointer tio) {
    //TODO vRince bind this with *actual* filter values ...
    m_filter[0] = new EngineFilterButterworth8(FILTER_LOWPASS, tio->getSampleRate(), 200);
    m_filter[1] = new EngineFilterButterworth8(FILTER_BANDPASS, tio->getSampleRate(), 200, 2000);
    m_filter[2] = new EngineFilterButterworth8(FILTER_HIGHPASS, tio->getSampleRate(), 2000);
}

void AnalyserWaveform::destroyFilters() {
    if( m_filter[0])
        delete m_filter[0];
    if( m_filter[1])
        delete m_filter[1];
    if( m_filter[2])
        delete m_filter[2];

    m_filter[0] = 0;
    m_filter[1] = 0;
    m_filter[2] = 0;
}

void AnalyserWaveform::process(const CSAMPLE *buffer, const int bufferLength) {
    if(!m_ready)
        return;

    //this should only append once
    if( bufferLength > buffers_[0].size()) {
        buffers_[0].resize(bufferLength);
        buffers_[1].resize(bufferLength);
        buffers_[2].resize(bufferLength);
    }

    m_filter[0]->process( buffer, &buffers_[0][0], bufferLength);
    m_filter[1]->process( buffer, &buffers_[1][0], bufferLength);
    m_filter[2]->process( buffer, &buffers_[2][0], bufferLength);

    for( int i = 0; i < bufferLength; i+=2) {
        //accumulate signal power of the stride
        m_currentStridePower[0] += buffer[i]*buffer[i];
        m_currentStridePower[1] += buffer[i+1]*buffer[i+1];
        m_currentStrideFiltredPower[0][0] += buffers_[0][i]*buffers_[0][i];
        m_currentStrideFiltredPower[1][0] += buffers_[0][i+1]*buffers_[0][i+1];
        m_currentStrideFiltredPower[0][1] += buffers_[1][i]*buffers_[1][i];
        m_currentStrideFiltredPower[1][1] += buffers_[1][i+1]*buffers_[1][i+1];
        m_currentStrideFiltredPower[0][2] += buffers_[2][i]*buffers_[2][i];
        m_currentStrideFiltredPower[1][2] += buffers_[2][i+1]*buffers_[2][i+1];

        if( m_strideBufferPos >= m_strideLength) {
            if( m_currentStride >= m_waveform->size()) {
                qDebug() << "m_currentStride >= m_power.size() This should never happend !";
                return;
            }

            storeCurentStridePower();
            resetCurrentStride();
            m_currentStride += 2;
        }
        m_strideBufferPos += 2;
    }
}

void AnalyserWaveform::storeCurentStridePower() {
    for( int i = 0; i < 2; i++) {
        m_waveform->all(m_currentStride+i) = (unsigned char)( m_convertionFactor * m_currentStridePower[i]);
        m_waveform->low(m_currentStride+i) = (unsigned char)( m_convertionFactor * m_currentStrideFiltredPower[i][0]);
        m_waveform->mid(m_currentStride+i) = (unsigned char)( m_convertionFactor * m_currentStrideFiltredPower[i][1]);
        m_waveform->high(m_currentStride+i) = (unsigned char)( m_convertionFactor * m_currentStrideFiltredPower[i][2]);
    }

}

void AnalyserWaveform::resetCurrentStride() {
    m_strideBufferPos = 0;
    for( int i = 0; i < 2; i++) {
        m_currentStridePower[i] = 0.0f;
        for( int f = 0; f < 3; f++) {
            m_currentStrideFiltredPower[i][f] = 0.0f;
        }
    }
}

void AnalyserWaveform::finalise(TrackPointer /*tio*/) {
    qDebug() << "Waveform gerenration done" << timer_->elapsed()/1000.0 << "s";
}
