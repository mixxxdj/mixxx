

#include <QtDebug>
#include <time.h>
#include <math.h>

#include "trackinfoobject.h"
#include "analyserrg.h"
#include "replaygain/replaygain_analysis.h"

AnalyserGain::AnalyserGain() {

}

void AnalyserGain::initialise(TrackPointer tio, int sampleRate, int totalSamples) {

   stepcontrol = 0;
    if(tio->getRG() != -32767) {
        return;
   }

    if(totalSamples == 0) return;
    stepcontrol = InitGainAnalysis( (long)sampleRate );

 //   m_iStartTime = clock();
}




void AnalyserGain::process(const CSAMPLE *pIn, const int iLen) {

	if(stepcontrol==0) return;
	//optimal: float_t m_fLems[4096], m_fRems[4096];
	float_t m_fLems[4096], m_fRems[4096];
	int RGCounter = 0;
	for(int i=0; i<iLen; i+=2) {
    		CSAMPLE l = pIn[i];
    		CSAMPLE r = pIn[i+1];
        	m_fLems[RGCounter] = (float_t)l*32767;
        	m_fRems[RGCounter] = (float_t)r*32767;

        	RGCounter++;
    }

	stepcontrol = AnalyzeSamples(m_fLems,m_fRems,RGCounter,2);
}




void AnalyserGain::finalise(TrackPointer tio) {

	if(stepcontrol==0) return;

	float_t Gain_Result = GetTitleGain();

	tio->setRG(Gain_Result);

    stepcontrol=0;
//m_iStartTime = clock() - m_iStartTime;
//qDebug() << "AnalyserGain :: Generation took " << double(m_iStartTime) / CLOCKS_PER_SEC << " seconds";
}

