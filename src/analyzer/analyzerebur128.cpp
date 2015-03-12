#include <analyzer/analyzerebur128.h>
#include <QtDebug>
#include <ebu_r128_proc.h>

#include "trackinfoobject.h"
#include "util/math.h"
#include "util/sample.h"

static const float kReplayGain2ReferenceLUFS = -18;

AnalyzerEbur128::AnalyzerEbur128(UserSettingsPointer config)
        : m_pConfig(config),
          m_iBufferSize(0) {
    m_pTempBuffer[0] = NULL;
    m_pTempBuffer[1] = NULL;
    m_pEbu128Proc = new Ebu_r128_proc();
}

AnalyzerEbur128::~AnalyzerEbur128() {
    delete [] m_pTempBuffer[0];
    delete [] m_pTempBuffer[1];
    delete m_pEbu128Proc;
}

bool AnalyzerEbur128::initialize(TrackPointer tio, int sampleRate, int totalSamples) {
    if (loadStored(tio) || totalSamples == 0) {
        return false;
    }

    m_pEbu128Proc->init (2, sampleRate);
    m_pEbu128Proc->integr_start ();
    return true;
}

bool AnalyzerEbur128::loadStored(TrackPointer tio) const {
    /*
    bool bAnalyserEnabled = m_pConfig->getValueString(
            ConfigKey("[ReplayGain]","ReplayGainAnalyserEnabled")).toInt() == 2;
    float fReplayGain = tio->getReplayGain();
    if (fReplayGain != 0 || !bAnalyserEnabled) {
        return true;
    }
    */
    return false;
}

void AnalyzerEbur128::cleanup(TrackPointer tio) {
    Q_UNUSED(tio);
}

void AnalyzerEbur128::process(const CSAMPLE *pIn, const int iLen) {
    int halfLength = static_cast<int>(iLen / 2);
    if (halfLength > m_iBufferSize) {
        delete [] m_pTempBuffer[0];
        delete [] m_pTempBuffer[1];
        m_pTempBuffer[0] = new CSAMPLE[halfLength];
        m_pTempBuffer[1] = new CSAMPLE[halfLength];
    }
    SampleUtil::deinterleaveBuffer(m_pTempBuffer[0], m_pTempBuffer[1], pIn, halfLength);
    m_pEbu128Proc->process(halfLength, m_pTempBuffer);
}

void AnalyzerEbur128::finalize(TrackPointer tio) {
    const float averageLufs = m_pEbu128Proc->integrated();
    const float fReplayGain2 = db2ratio(kReplayGain2ReferenceLUFS - averageLufs);
    qDebug() << "ReplayGain2 result is" << averageLufs << "LUFS RG2:" << fReplayGain2 << "for" << tio->getFileName();

    Mixxx::ReplayGain replayGain(tio->getReplayGain());
    replayGain.setRatio(fReplayGain2);
    tio->setReplayGain(replayGain);
}
