

#include <QtDebug>
#include <time.h>
#include <math.h>

#include "trackinfoobject.h"
#include "analyserrg.h"
#include "replaygain/replaygain_analysis.h"

AnalyserGain::AnalyserGain(ConfigObject<ConfigValue> *_config) {
	 m_pConfigRG = _config;
	 m_istepcontrol = 0;
}
//TODO: Rewriting replaygain/replagain_analys.* may improve performances. Anyway those willing to do should be sure of
//		the resulting values to exactly coincide with "classical" replaygain_analysis.* ones.
//		On the other hand, every other ReplayGain tagger uses exactly these methods and we do not have problems about
//		values to coincide.

void AnalyserGain::initialise(TrackPointer tio, int sampleRate, int totalSamples) {

	bool AnalyserEnabled = (bool)m_pConfigRG->getValueString(ConfigKey("[ReplayGain]","ReplayGainAnalyserEnabled")).toInt();
	float RG = tio->getRG();
	if(totalSamples == 0 || RG != 0 || !AnalyserEnabled) {
		qDebug() << "Replaygain Analyser will not start.";
		if (RG != 0 ) qDebug() << "Found a ReplayGain value of " << 20*log10(RG) << "dB for track :" <<(tio->getFilename());
		return;
	}
	m_istepcontrol = InitGainAnalysis( (long)sampleRate );

 //   m_iStartTime = clock();
}




void AnalyserGain::process(const CSAMPLE *pIn, const int iLen) {

	if(m_istepcontrol!=1) return;

	CSAMPLE *m_fLems = new CSAMPLE[4096];
	CSAMPLE *m_fRems = new CSAMPLE[4096];
	int RGCounter = 0;
	for(int i=0; i<iLen; i+=2) {
        	m_fLems[RGCounter] = pIn[i]*32767;
        	m_fRems[RGCounter] = pIn[i+1]*32767;

        	RGCounter++;
    }

	m_istepcontrol = AnalyzeSamples(m_fLems,m_fRems,RGCounter,2);
}




void AnalyserGain::finalise(TrackPointer tio) {

	if(m_istepcontrol!=1) return;

	//TODO: Digg into replay_gain code and modify it so that
	// it directly sends the result as relative peaks.
	// In that way there is no need to do this:

	float_t Gain_Result = pow(10,GetTitleGain()/20);

	tio->setRG(Gain_Result);
	if(Gain_Result) qDebug() << "ReplayGain Analyser found a ReplayGain value of "<< 20*log10(Gain_Result) << "dB for track " << (tio->getFilename());
	m_istepcontrol=0;
	Gain_Result=0;
//m_iStartTime = clock() - m_iStartTime;
//qDebug() << "AnalyserGain :: Generation took " << double(m_iStartTime) / CLOCKS_PER_SEC << " seconds";
}

