#ifndef ANALYSER_WAVEFORM_H
#define ANALYSER_WAVEFORM_H

#include "analyser.h"

class EngineFilterButterworth8;
class Waveform;

class QTime;

class AnalyserWaveform : public Analyser {

public:
    AnalyserWaveform();
    virtual ~AnalyserWaveform();

    void initialise(TrackPointer tio, int sampleRate, int totalSamples);

    void process(const CSAMPLE *pIn, const int iLen);
    void finalise(TrackPointer tio);

private:
    void storeCurentStridePower();
    void resetCurrentStride();

    void resetFilters(TrackPointer tio);
    void destroyFilters();

private:
    Waveform* m_waveform;

    // Current stride
    int m_currentStride;
    float m_timePerStride;
    int m_strideLength;
    int m_strideBufferPos;
    float m_currentStridePower[2];
    float m_currentStrideFiltredPower[2][3];
    float m_convertionFactor;
    bool m_ready;

    EngineFilterButterworth8* m_filter[3];

    QTime* timer_;
};

#endif
