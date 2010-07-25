#ifndef ANALYSER_WAVESUMMARY_H
#define ANALYSER_WAVESUMMARY_H

#include <QByteArray>

#include "analyser.h"

class AnalyserWavesummary : public Analyser {

  public:
    AnalyserWavesummary();
    void initialise(TrackPointer tio, int sampleRate, int totalSamples);
    void process(const CSAMPLE *pIn, const int iLen);
    void finalise(TrackPointer tio);

  private:
    QByteArray *m_pData;
    int m_iCurPos;
    int m_iBufferPos;
    int m_iStrideLength;
    float m_fMax;
    float m_fMin;
};

#endif
