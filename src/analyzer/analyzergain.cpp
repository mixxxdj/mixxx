#include <QtDebug>
#include <replaygain.h>

#include "analyzer/analyzergain.h"
#include "track/track.h"
#include "util/math.h"
#include "util/sample.h"
#include "util/timer.h"

AnalyzerGain::AnalyzerGain(UserSettingsPointer pConfig)
    : m_initalized(false),
      m_rgSettings(pConfig),
      m_pLeftTempBuffer(NULL),
      m_pRightTempBuffer(NULL),
      m_iBufferSize(0) {
    m_pReplayGain = new ReplayGain();
}

AnalyzerGain::~AnalyzerGain() {
    delete [] m_pLeftTempBuffer;
    delete [] m_pRightTempBuffer;
    delete m_pReplayGain;
}

bool AnalyzerGain::initialize(TrackPointer tio, int sampleRate, int totalSamples) {
    if (isDisabledOrLoadStoredSuccess(tio) || totalSamples == 0) {
        return false;
    }

    m_initalized = m_pReplayGain->initialise((long)sampleRate, 2);
    return true;
}

bool AnalyzerGain::isDisabledOrLoadStoredSuccess(TrackPointer tio) const {
    return m_rgSettings.isAnalyzerDisabled(1, tio);
}

void AnalyzerGain::cleanup(TrackPointer tio) {
    m_initalized = false;
    Q_UNUSED(tio);
}

void AnalyzerGain::process(const CSAMPLE *pIn, const int iLen) {
    if (!m_initalized) {
        return;
    }
    ScopedTimer t("AnalyzerGain::process()");

    int halfLength = static_cast<int>(iLen / 2);
    if (halfLength > m_iBufferSize) {
        delete [] m_pLeftTempBuffer;
        delete [] m_pRightTempBuffer;
        m_pLeftTempBuffer = new CSAMPLE[halfLength];
        m_pRightTempBuffer = new CSAMPLE[halfLength];
    }
    SampleUtil::deinterleaveBuffer(m_pLeftTempBuffer, m_pRightTempBuffer, pIn, halfLength);
    SampleUtil::applyGain(m_pLeftTempBuffer, 32767, halfLength);
    SampleUtil::applyGain(m_pRightTempBuffer, 32767, halfLength);
    m_initalized = m_pReplayGain->process(m_pLeftTempBuffer, m_pRightTempBuffer, halfLength);
}

void AnalyzerGain::finalize(TrackPointer tio) {
    //TODO: We are going to store values as relative peaks so that "0" means that no replaygain has been evaluated.
    // This means that we are going to transform from dB to peaks and viceversa.
    // One may think to digg into replay_gain code and modify it so that
    // it directly sends results as relative peaks.
    // In that way there is no need to spend resources in calculating log10 or pow.
    if(!m_initalized)
        return;

    float fReplayGainOutput = m_pReplayGain->end();
    if (fReplayGainOutput == GAIN_NOT_ENOUGH_SAMPLES) {
        qDebug() << "ReplayGain 1.0 analysis failed";
        m_initalized = false;
        return;
    }

    mixxx::ReplayGain replayGain(tio->getReplayGain());
    replayGain.setRatio(db2ratio(fReplayGainOutput));
    tio->setReplayGain(replayGain);
    qDebug() << "ReplayGain 1.0 result is" << fReplayGainOutput << "dB for" << tio->getLocation();
    m_initalized = false;
}
