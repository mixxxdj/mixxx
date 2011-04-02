#ifndef ANALYSER_WAVEFORM_H
#define ANALYSER_WAVEFORM_H

#include "analyser.h"

class EngineFilterButterworth8;
class Waveform;

class AnalyserWaveform : public Analyser {

public:
    AnalyserWaveform();
    virtual ~AnalyserWaveform();

    void initialise(TrackPointer tio, int sampleRate, int totalSamples);

    void process(const CSAMPLE *pIn, const int iLen);
    void finalise(TrackPointer tio);

    //Downsample finding the maximum of each stride buffer
    void process_old(const CSAMPLE *pIn, const int iLen);

private:
    void storeCurentStridePower();
    void resetCurrentStride();

private:
    QVector<float> *downsample;
    float *downsampleVector;

    //kind of weaid timing ...
    int m_iStartTime;

    int m_iStrideLength; //downsample length
    int m_iSamplePos; //position in the samples
    int m_iStrideBufferPos; //position in the stride

    float m_timePerStride;

    //Use to find max
    float m_fLMax;
    float m_fRMax;
    float m_fOldLMax;
    float m_fOldRMax;

    //////////////
    Waveform* m_waveform;

    // Current stride
    int m_currentStride;
    float m_currentStridePower[2];
    float m_currentStrideFiltredPower[2][3];

    bool m_ready;

    EngineFilterButterworth8* m_filter[3];
};

#endif
