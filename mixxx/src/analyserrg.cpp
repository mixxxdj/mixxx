

#include <QtDebug>
#include <time.h>
#include <math.h>

#include "trackinfoobject.h"
#include "analyserrg.h"
#include "../lib/replaygain/replaygain_analysis.h"

AnalyserGain::AnalyserGain(ConfigObject<ConfigValue> *_config) {
    m_pConfigReplayGain = _config;
    m_iStepControl = 0;
    m_pLeftTempBuffer = NULL;
    m_pRightTempBuffer = NULL;
    m_iBufferSize = 0;
}

AnalyserGain::~AnalyserGain() {
    delete [] m_pLeftTempBuffer;
    delete [] m_pRightTempBuffer;
}

//TODO: On may think on rewriting replaygain/replagain_analys.* to improve performances. Anyway those willing to do should be sure of
//		the resulting values to exactly coincide with "classical" replaygain_analysis.* ones.
//		On the other hand, every other ReplayGain tagger uses exactly these methods so that we do not have problems about
//		values to coincide.

bool AnalyserGain::initialise(TrackPointer tio, int sampleRate, int totalSamples) {

    bool bAnalyserEnabled = (bool)m_pConfigReplayGain->getValueString(ConfigKey("[ReplayGain]","ReplayGainAnalyserEnabled")).toInt();
    float fReplayGain = tio->getReplayGain();
    if(totalSamples == 0 || fReplayGain != 0 || !bAnalyserEnabled) {
        //qDebug() << "Replaygain Analyser will not start.";
        //if (fReplayGain != 0 ) qDebug() << "Found a ReplayGain value of " << 20*log10(fReplayGain) << "dB for track :" <<(tio->getFilename());
        return false;
    }
    m_iStepControl = InitGainAnalysis( (long)sampleRate );
    return true;
}

void AnalyserGain::process(const CSAMPLE *pIn, const int iLen) {
    if(m_iStepControl != 1)
        return;

    int halfLength = static_cast<int>(iLen / 2);
    if (halfLength > m_iBufferSize) {
        delete [] m_pLeftTempBuffer;
        delete [] m_pRightTempBuffer;
        m_pLeftTempBuffer = new CSAMPLE[halfLength];
        m_pRightTempBuffer = new CSAMPLE[halfLength];
    }

    for (int i = 0; i < halfLength; ++i) {
        m_pLeftTempBuffer[i] = pIn[i*2] * 32767;
        m_pRightTempBuffer[i] = pIn[i*2+1] * 32767;
    }
    m_iStepControl = AnalyzeSamples(m_pLeftTempBuffer, m_pRightTempBuffer,
                                    halfLength, 2);
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
