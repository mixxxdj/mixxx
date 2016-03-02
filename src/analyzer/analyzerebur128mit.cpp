#include <analyzer/analyzerebur128mit.h>
#include <QtDebug>

#include "trackinfoobject.h"
#include "util/math.h"
#include "util/sample.h"
#include "util/timer.h"

namespace {
const double kReplayGain2ReferenceLUFS = -18;
} // anonymous namespace

AnalyzerEbur128Mit::AnalyzerEbur128Mit(UserSettingsPointer pConfig)
        : m_pConfig(pConfig),
          m_initalized(false),
          m_iBufferSize(0),
          m_pState(nullptr) {
}

AnalyzerEbur128Mit::~AnalyzerEbur128Mit() {
}

bool AnalyzerEbur128Mit::initialize(TrackPointer tio,
        int sampleRate, int totalSamples) {
    if (isDisabledOrLoadStoredSuccess(tio) || totalSamples == 0) {
        return false;
    }

    m_pState = ebur128_init(2u,
            static_cast<unsigned long>(sampleRate),
            EBUR128_MODE_I);

    m_initalized = nullptr != m_pState;
    return m_initalized;
}

bool AnalyzerEbur128Mit::isDisabledOrLoadStoredSuccess(TrackPointer tio) const {
    // WARNING: Do not fix the "analyser" spelling here since user config files
    // contain these strings.
    int version = m_pConfig->getValueString(
            ConfigKey("[ReplayGain]", "ReplayGainAnalyserVersion")).toInt();
    bool analyzerEnabled = ((bool)m_pConfig->getValueString(
            ConfigKey("[ReplayGain]", "ReplayGainAnalyserEnabled")).toInt()) &&
            (version == 2);
    bool reanalyse = m_pConfig->getValueString(
            ConfigKey("[ReplayGain]", "ReplayGainReanalyze")).toInt();

    if (analyzerEnabled) {
        if (reanalyse) {
            return false;
        }
        return tio->getReplayGain().hasRatio();
    }
    return true;
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
    size_t frames = iLen / 2;
    int e = ebur128_add_frames_float(m_pState, pIn, frames);
    DEBUG_ASSERT(e == EBUR128_SUCCESS);
}

void AnalyzerEbur128Mit::finalize(TrackPointer tio) {
    if (!m_initalized) {
        return;
    }
    double averageLufs;
    int e = ebur128_loudness_global(m_pState, &averageLufs);
    cleanup(tio);
    if (e != EBUR128_SUCCESS) {
        qDebug() << "AnalyzerEbur128Mit::finalize failed with" << e;
        return;
    }
    if (averageLufs == -HUGE_VAL || averageLufs == 0.0) {
        qWarning() << "AnalyzerEbur128Mit::finalize averageLufs invalid:"
                 << averageLufs;
        return;
    }

    const double fReplayGain2 = kReplayGain2ReferenceLUFS - averageLufs;
    Mixxx::ReplayGain replayGain(tio->getReplayGain());
    replayGain.setRatio(db2ratio(fReplayGain2));
    tio->setReplayGain(replayGain);
    qDebug() << "ReplayGain 2.0 (MIT) result is" << fReplayGain2 << "dB for" << tio->getLocation();
}
