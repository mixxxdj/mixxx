#ifndef ANALYSER_WAVEFORM_H
#define ANALYSER_WAVEFORM_H

#include "analyser.h"

class AnalyserWaveform : public Analyser {

  public:
    AnalyserWaveform();
    void initialise(TrackPointer tio, int sampleRate, int totalSamples);
    void process(const CSAMPLE *pIn, const int iLen);
    void finalise(TrackPointer tio);

  private:
    QVector<float> *downsample;
    float *downsampleVector;

    int m_iStartTime;
    int m_iStrideLength;
    int m_iCurPos;
    int m_iBufferPos;
    float m_fLMax;
    float m_fRMax;
    float m_fOldLMax;
    float m_fOldRMax;
};

#endif
