

#include <QtDebug>
#include <time.h>
#include <math.h>

#include "trackinfoobject.h"
#include "analyserrg.h"
#include "replaygain/replaygain_analysis.h"

AnalyserGain::AnalyserGain(ConfigObject<ConfigValue> *_config) {
    m_pConfigRG = _config;
    m_iStepControl = 0;
}
//TODO: Rewriting replaygain/replagain_analys.* may improve performances. Anyway those willing to do should be sure of
//		the resulting values to exactly coincide with "classical" replaygain_analysis.* ones.
//		On the other hand, every other ReplayGain tagger uses exactly these methods and we do not have problems about
//		values to coincide.

void AnalyserGain::initialise(TrackPointer tio, int sampleRate, int totalSamples) {

	bool bAnalyserEnabled = (bool)m_pConfigRG->getValueString(ConfigKey("[ReplayGain]","ReplayGainAnalyserEnabled")).toInt();
	float fRG = tio->getRG();
	if(totalSamples == 0 || fRG != 0 || !bAnalyserEnabled) {
		qDebug() << "Replaygain Analyser will not start.";
		if (fRG != 0 ) qDebug() << "Found a ReplayGain value of " << 20*log10(fRG) << "dB for track :" <<(tio->getFilename());
		return;
	}
	m_iStepControl = InitGainAnalysis( (long)sampleRate );

	//   m_iStartTime = clock();
}




void AnalyserGain::process(const CSAMPLE *pIn, const int iLen) {

	if(m_iStepControl!=1) return;

	CSAMPLE *m_fLems = new CSAMPLE[(int)(iLen/2)];
	CSAMPLE *m_fRems = new CSAMPLE[(int)(iLen/2)];
	int iRGCounter = 0;
	for(int i=0; i<iLen; i+=2) {
		m_fLems[iRGCounter] = pIn[i]*32767;
		m_fRems[iRGCounter] = pIn[i+1]*32767;

		iRGCounter++;
	}

	m_iStepControl = AnalyzeSamples(m_fLems,m_fRems,iRGCounter,2);

	delete m_fLems;
	delete m_fRems;

}




void AnalyserGain::finalise(TrackPointer tio) {

	if(m_iStepControl!=1) return;

	//TODO: Digg into replay_gain code and modify it so that
	// it directly sends the result as relative peaks.
	// In that way there is no need to do this:

	float fGain_Result = pow(10,GetTitleGain()/20);
	tio->setRG(fGain_Result);
	if(fGain_Result) qDebug() << "ReplayGain Analyser found a ReplayGain value of "<< 20*log10(fGain_Result) << "dB for track " << (tio->getFilename());
	m_iStepControl=0;
	fGain_Result=0;
	//m_iStartTime = clock() - m_iStartTime;
	//qDebug() << "AnalyserGain :: Generation took " << double(m_iStartTime) / CLOCKS_PER_SEC << " seconds";
}

