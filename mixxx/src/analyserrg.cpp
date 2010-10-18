

#include <QtDebug>
#include <time.h>
#include <math.h>

#include "trackinfoobject.h"
#include "analyserrg.h"
#include "replaygain/replaygain_analysis.h"

AnalyserGain::AnalyserGain() {

}

void AnalyserGain::initialise(TrackPointer tio, int sampleRate, int totalSamples) {

	m_istepcontrol = 0;
    if(tio->getRG() != 0) {
        return;
   }

    if(totalSamples == 0) return;
    m_istepcontrol = InitGainAnalysis( (long)sampleRate );

 //   m_iStartTime = clock();
}




void AnalyserGain::process(const CSAMPLE *pIn, const int iLen) {

	if(m_istepcontrol!=1) return;
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

	m_istepcontrol = AnalyzeSamples(m_fLems,m_fRems,RGCounter,2);
}




void AnalyserGain::finalise(TrackPointer tio) {

	if(m_istepcontrol!=1) return;

	//TODO: Digg into replay_gain code and modify it so that
	// it directly sends the result as relative amplitude.
	// so that there is no need to do this:

	float_t Gain_Result = pow(10,GetTitleGain()/20);

	tio->setRG(Gain_Result);

	m_istepcontrol=0;
//m_iStartTime = clock() - m_iStartTime;
//qDebug() << "AnalyserGain :: Generation took " << double(m_iStartTime) / CLOCKS_PER_SEC << " seconds";
}

