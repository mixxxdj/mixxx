#include <analyzer/analyzerebur128.h>
#include <QtDebug>
#include <ebu_r128_proc.h>

#include "trackinfoobject.h"
#include "util/math.h"
#include "util/sample.h"
#include "util/timer.h"

static const float kReplayGain2ReferenceLUFS = -18;

AnalyzerEbur128::AnalyzerEbur128(UserSettingsPointer config)
        : m_pConfig(config),
          m_initalized(false),
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
    m_initalized = true;
    return true;
}

bool AnalyzerEbur128::loadStored(TrackPointer tio) const {
    bool bAnalyserEnabled = (bool)m_pConfig->getValueString(
            ConfigKey("[ReplayGain]","ReplayGainAnalyserEnabled")).toInt();
    if (/*tio->getReplayGain().hasRatio() ||*/ !bAnalyserEnabled) {
        return true;
    }
    return false;
}

void AnalyzerEbur128::cleanup(TrackPointer tio) {
    Q_UNUSED(tio);
    m_pEbu128Proc->reset();
    m_initalized = false;
}

void AnalyzerEbur128::process(const CSAMPLE *pIn, const int iLen) {
    if (!m_initalized) {
        return;
    }
    ScopedTimer t("AnalyserEbur128::process()");
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
    if (!m_initalized) {
        return;
    }
    const float averageLufs = m_pEbu128Proc->integrated();
    const float fReplayGain2 = kReplayGain2ReferenceLUFS - averageLufs;
    Mixxx::ReplayGain replayGain(tio->getReplayGain());
    replayGain.setRatio(db2ratio(fReplayGain2));
    tio->setReplayGain(replayGain);
    qDebug() << "ReplayGain2 result is" << fReplayGain2 << "dB for" << tio->getLocation();
}
