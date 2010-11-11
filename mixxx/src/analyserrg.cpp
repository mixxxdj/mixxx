

#include <QtDebug>
#include <time.h>
#include <math.h>

#include "trackinfoobject.h"
#include "analyserrg.h"
#include "../lib/replaygain/replaygain_analysis.h"

AnalyserGain::AnalyserGain(ConfigObject<ConfigValue> *_config) {
    m_pConfigReplayGain = _config;
    m_iStepControl = 0;
}
//TODO: On may think on rewriting replaygain/replagain_analys.* to improve performances. Anyway those willing to do should be sure of
//		the resulting values to exactly coincide with "classical" replaygain_analysis.* ones.
//		On the other hand, every other ReplayGain tagger uses exactly these methods so that we do not have problems about
//		values to coincide.

void AnalyserGain::initialise(TrackPointer tio, int sampleRate, int totalSamples) {

    bool bAnalyserEnabled = (bool)m_pConfigReplayGain->getValueString(ConfigKey("[ReplayGain]","ReplayGainAnalyserEnabled")).toInt();
    float fReplayGain = tio->getReplayGain();
    if(totalSamples == 0 || fReplayGain != 0 || !bAnalyserEnabled) {
        //qDebug() << "Replaygain Analyser will not start.";
        //if (fReplayGain != 0 ) qDebug() << "Found a ReplayGain value of " << 20*log10(fReplayGain) << "dB for track :" <<(tio->getFilename());
        return;
    }
    m_iStepControl = InitGainAnalysis( (long)sampleRate );

    //   m_iStartTime = clock();
}




void AnalyserGain::process(const CSAMPLE *pIn, const int iLen) {

    if(m_iStepControl!=1) return;

    CSAMPLE *m_fLeftChannel = new CSAMPLE[(int)(iLen/2)];
    CSAMPLE *m_fRightChannel = new CSAMPLE[(int)(iLen/2)];
    int iRGCounter = 0;
    for(int i=0; i<iLen; i+=2) {
        m_fLeftChannel[iRGCounter] = pIn[i]*32767;
        m_fRightChannel[iRGCounter] = pIn[i+1]*32767;

        iRGCounter++;
    }

    m_iStepControl = AnalyzeSamples(m_fLeftChannel,m_fRightChannel,iRGCounter,2);

    delete m_fLeftChannel;
    delete m_fRightChannel;
    m_fLeftChannel = NULL;
    m_fRightChannel = NULL;

}




void AnalyserGain::finalise(TrackPointer tio) {

    if(m_iStepControl!=1) return;

    //TODO: We are going to store values as relative peaks so that "0" means that no replaygain has been evaluated.
    // This means that we are going to transform from dB to peaks and viceversa.
    // One may think to digg into replay_gain code and modify it so that
    // it directly sends results as relative peaks.
    // In that way there is no need to spend resources in calculating log10 or pow.

    float fReplayGain_Result = pow(10,GetTitleGain()/20);
    tio->setReplayGain(fReplayGain_Result);
    //if(fReplayGain_Result) qDebug() << "ReplayGain Analyser found a ReplayGain value of "<< 20*log10(fReplayGain_Result) << "dB for track " << (tio->getFilename());
    m_iStepControl=0;
    fReplayGain_Result=0;
    //m_iStartTime = clock() - m_iStartTime;
    //qDebug() << "AnalyserGain :: Generation took " << double(m_iStartTime) / CLOCKS_PER_SEC << " seconds";
}
