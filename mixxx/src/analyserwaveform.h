#ifndef ANALYSER_WAVEFORM_H
#define ANALYSER_WAVEFORM_H

#include "analyser.h"

class AnalyserWaveform : public Analyser {

public:
	AnalyserWaveform();
	void initialise(TrackInfoObject* tio, int sampleRate, int totalSamples);
	void process(const CSAMPLE *pIn, const int iLen);
	void finalise(TrackInfoObject* tio);

private:
	QVector<float> *downsample;
    float *downsampleVector;
    
    int m_iStartTime;
    int m_iStrideLength;
    int m_iCurPos;
    int m_iBufferPos;
    float m_fLMax;
    float m_fRMax;
};

#endif
