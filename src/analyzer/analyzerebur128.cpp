#include "analyzer/analyzerebur128.h"

#include <QtDebug>

#include "track/track.h"
#include "util/math.h"
#include "util/sample.h"
#include "util/timer.h"

namespace {
const double kReplayGain2ReferenceLUFS = -18;
} // anonymous namespace

AnalyzerEbur128::AnalyzerEbur128(UserSettingsPointer pConfig)
        : m_rgSettings(pConfig),
          m_pState(nullptr) {
}

AnalyzerEbur128::~AnalyzerEbur128() {
    cleanup(); // ...to prevent memory leaks
}

bool AnalyzerEbur128::initialize(TrackPointer tio,
        int sampleRate,
        int totalSamples) {
    if (m_rgSettings.isAnalyzerDisabled(2, tio) || totalSamples == 0) {
        qDebug() << "Skipping AnalyzerEbur128";
        return false;
    }
    DEBUG_ASSERT(m_pState == nullptr);
    m_pState = ebur128_init(2u,
            static_cast<unsigned long>(sampleRate),
            EBUR128_MODE_I);
    return m_pState != nullptr;
}

void AnalyzerEbur128::cleanup() {
    if (m_pState) {
        ebur128_destroy(&m_pState);
        // ebur128_destroy clears the pointer but let's not rely on that.
        m_pState = nullptr;
    }
}

bool AnalyzerEbur128::processSamples(const CSAMPLE *pIn, const int iLen) {
    VERIFY_OR_DEBUG_ASSERT(m_pState) {
        return false;
    }
    ScopedTimer t("AnalyzerEbur128::processSamples()");
    size_t frames = iLen / 2;
    int e = ebur128_add_frames_float(m_pState, pIn, frames);
    VERIFY_OR_DEBUG_ASSERT(e == EBUR128_SUCCESS) {
        qWarning() << "AnalyzerEbur128::processSamples() failed with" << e;
        return false;
    }
    return true;
}

void AnalyzerEbur128::storeResults(TrackPointer tio) {
    VERIFY_OR_DEBUG_ASSERT(m_pState) {
        return;
    }
    double averageLufs;
    int e = ebur128_loudness_global(m_pState, &averageLufs);
    VERIFY_OR_DEBUG_ASSERT(e == EBUR128_SUCCESS) {
        qWarning() << "AnalyzerEbur128::storeResults() failed with" << e;
        return;
    }
    if (averageLufs == -HUGE_VAL || averageLufs == 0.0) {
        qWarning() << "AnalyzerEbur128::storeResults() averageLufs invalid:"
                   << averageLufs;
        return;
    }

    const double fReplayGain2 = kReplayGain2ReferenceLUFS - averageLufs;
    mixxx::ReplayGain replayGain(tio->getReplayGain());
    replayGain.setRatio(db2ratio(fReplayGain2));
    tio->setReplayGain(replayGain);
    qDebug() << "ReplayGain 2.0 (libebur128) result is" << fReplayGain2 << "dB for" << tio->getFileInfo();
}
