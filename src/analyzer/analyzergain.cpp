#include "analyzer/analyzergain.h"

#include <replaygain.h>

#include <QtDebug>

#include "analyzer/analyzertrack.h"
#include "analyzer/constants.h"
#include "track/track.h"
#include "util/math.h"
#include "util/sample.h"
#include "util/timer.h"

AnalyzerGain::AnalyzerGain(UserSettingsPointer pConfig)
        : m_rgSettings(pConfig) {
    m_pReplayGain = new ReplayGain();
}

AnalyzerGain::~AnalyzerGain() {
    delete m_pReplayGain;
}

bool AnalyzerGain::initialize(const AnalyzerTrack& track,
        mixxx::audio::SampleRate sampleRate,
        SINT frameLength) {
    if (m_rgSettings.isAnalyzerDisabled(1, track.getTrack()) || frameLength <= 0) {
        qDebug() << "Skipping AnalyzerGain";
        return false;
    }

    return m_pReplayGain->initialise(
            sampleRate,
            mixxx::kAnalysisChannels);
}

void AnalyzerGain::cleanup() {
}

bool AnalyzerGain::processSamples(const CSAMPLE* pIn, SINT count) {
    ScopedTimer t(u"AnalyzerGain::process()");

    SINT numFrames = count / mixxx::kAnalysisChannels;
    if (numFrames > static_cast<SINT>(m_pLeftTempBuffer.size())) {
        m_pLeftTempBuffer.resize(numFrames);
        m_pRightTempBuffer.resize(numFrames);
    }
    SampleUtil::deinterleaveBuffer(m_pLeftTempBuffer.data(),
            m_pRightTempBuffer.data(),
            pIn,
            numFrames);
    SampleUtil::applyGain(m_pLeftTempBuffer.data(), 32767, numFrames);
    SampleUtil::applyGain(m_pRightTempBuffer.data(), 32767, numFrames);
    return m_pReplayGain->process(m_pLeftTempBuffer.data(), m_pRightTempBuffer.data(), numFrames);
}

void AnalyzerGain::storeResults(TrackPointer pTrack) {
    //TODO: We are going to store values as relative peaks so that "0" means that no replaygain has been evaluated.
    // This means that we are going to transform from dB to peaks and vice-versa.
    // One may think to digg into replay_gain code and modify it so that
    // it directly sends results as relative peaks.
    // In that way there is no need to spend resources in calculating log10 or pow.

    float fReplayGainOutput = m_pReplayGain->end();
    if (fReplayGainOutput == GAIN_NOT_ENOUGH_SAMPLES) {
        qWarning() << "ReplayGain 1.0 analysis failed";
        return;
    }

    mixxx::ReplayGain replayGain(pTrack->getReplayGain());
    replayGain.setRatio(db2ratio(fReplayGainOutput));
    pTrack->setReplayGain(replayGain);
    qDebug() << "ReplayGain 1.0 result is" << fReplayGainOutput << "dB for"
             << pTrack->getLocation();
}
