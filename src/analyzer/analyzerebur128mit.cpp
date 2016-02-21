#include <analyzer/analyzerebur128mit.h>
#include <QtDebug>

#include "trackinfoobject.h"
#include "util/math.h"
#include "util/sample.h"
#include "util/timer.h"

static const float kReplayGain2ReferenceLUFS = -18;

AnalyzerEbur128Mit::AnalyzerEbur128Mit(UserSettingsPointer config)
        : m_pConfig(config),
          m_initalized(false),
          m_iBufferSize(0),
          m_pState(nullptr) {
}

AnalyzerEbur128Mit::~AnalyzerEbur128Mit() {
}

bool AnalyzerEbur128Mit::initialize(TrackPointer tio,
        int sampleRate, int totalSamples) {
    if (loadStored(tio) || totalSamples == 0) {
        return false;
    }

    m_pState = ebur128_init(2u,
            static_cast<unsigned int>(sampleRate),
            EBUR128_MODE_I);

    m_initalized = true;
    return true;
}

bool AnalyzerEbur128Mit::loadStored(TrackPointer tio) const {
    bool bAnalyserEnabled = (bool)m_pConfig->getValueString(
            ConfigKey("[ReplayGain]","ReplayGainAnalyserEnabled")).toInt();
    if (/*tio->getReplayGain().hasRatio() ||*/ !bAnalyserEnabled) {
        return true;
    }
    return false;
}

void AnalyzerEbur128Mit::cleanup(TrackPointer tio) {
    Q_UNUSED(tio);
    ebur128_destroy(&m_pState);
    m_initalized = false;
}

void AnalyzerEbur128Mit::process(const CSAMPLE *pIn, const int iLen) {
    if (!m_initalized) {
        return;
    }
    ScopedTimer t("AnalyserEbur128Mit::process()");
    int e = ebur128_add_frames_float(m_pState, pIn, static_cast<size_t>(iLen));
    DEBUG_ASSERT(e = EBUR128_SUCCESS);
}

void AnalyzerEbur128Mit::finalize(TrackPointer tio) {
    if (!m_initalized) {
        return;
    }
    double averageLufs;
    if (ebur128_loudness_global(m_pState, &averageLufs) != EBUR128_SUCCESS) {
        return;
    }
    const float fReplayGain2 = kReplayGain2ReferenceLUFS - averageLufs;
    Mixxx::ReplayGain replayGain(tio->getReplayGain());
    replayGain.setRatio(db2ratio(fReplayGain2));
    tio->setReplayGain(replayGain);
    qDebug() << "ReplayGain2 (MIT) result is" << fReplayGain2 << "dB for" << tio->getLocation();
}
