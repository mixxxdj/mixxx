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
        int sampleRate, int totalSamples) {
    if (isDisabledOrLoadStoredSuccess(tio) || totalSamples == 0) {
        return false;
    }
    if (!isInitialized()) {
        m_pState = ebur128_init(2u,
                static_cast<unsigned long>(sampleRate),
                EBUR128_MODE_I);
    }
    return isInitialized();
}

bool AnalyzerEbur128::isDisabledOrLoadStoredSuccess(TrackPointer tio) const {
    return m_rgSettings.isAnalyzerDisabled(2, tio);
}

void AnalyzerEbur128::cleanup() {
    if (isInitialized()) {
        ebur128_destroy(&m_pState);
        // ebur128_destroy clears the pointer but let's not rely on that.
        m_pState = nullptr;
    }
    DEBUG_ASSERT(!isInitialized());
}

void AnalyzerEbur128::cleanup(TrackPointer tio) {
    Q_UNUSED(tio);
    cleanup();
}

void AnalyzerEbur128::process(const CSAMPLE *pIn, const int iLen) {
    if (!isInitialized()) {
        return;
    }
    ScopedTimer t("AnalyzerEbur128::process()");
    size_t frames = iLen / 2;
    int e = ebur128_add_frames_float(m_pState, pIn, frames);
    DEBUG_ASSERT_AND_HANDLE(e == EBUR128_SUCCESS) {
        qWarning() << "AnalyzerEbur128::process() failed with" << e;
        return;
    }
}

void AnalyzerEbur128::finalize(TrackPointer tio) {
    if (!isInitialized()) {
        return;
    }
    double averageLufs;
    int e = ebur128_loudness_global(m_pState, &averageLufs);
    cleanup(tio);
    DEBUG_ASSERT_AND_HANDLE(e == EBUR128_SUCCESS) {
        qWarning() << "AnalyzerEbur128::finalize() failed with" << e;
        return;
    }
    if (averageLufs == -HUGE_VAL || averageLufs == 0.0) {
        qWarning() << "AnalyzerEbur128::finalize() averageLufs invalid:"
                 << averageLufs;
        return;
    }

    const double fReplayGain2 = kReplayGain2ReferenceLUFS - averageLufs;
    mixxx::ReplayGain replayGain(tio->getReplayGain());
    replayGain.setRatio(db2ratio(fReplayGain2));
    tio->setReplayGain(replayGain);
    qDebug() << "ReplayGain 2.0 (libebur128) result is" << fReplayGain2 << "dB for" << tio->getLocation();
}
