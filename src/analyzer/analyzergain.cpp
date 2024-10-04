#include "analyzer/analyzergain.h"

#include <replaygain.h>

#include <QtDebug>

#include "analyzer/analyzertrack.h"
#include "analyzer/constants.h"
#include "track/track.h"
#include "util/sample.h"
#include "util/timer.h"

AnalyzerGain::AnalyzerGain(UserSettingsPointer pConfig)
        : m_rgSettings(pConfig),
          m_pReplayGain(std::make_unique<ReplayGain>()) {
}

AnalyzerGain::~AnalyzerGain() = default;

bool AnalyzerGain::initialize(const AnalyzerTrack& track,
        mixxx::audio::SampleRate sampleRate,
        mixxx::audio::ChannelCount channelCount,
        SINT frameLength) {
    if (m_rgSettings.isAnalyzerDisabled(1, track.getTrack()) || frameLength <= 0) {
        qDebug() << "Skipping AnalyzerGain";
        return false;
    }
    m_channelCount = channelCount;

    return m_pReplayGain->initialise(
            sampleRate,
            mixxx::kAnalysisChannels);
}

void AnalyzerGain::cleanup() {
}

bool AnalyzerGain::processSamples(const CSAMPLE* pIn, SINT count) {
    ScopedTimer t(QStringLiteral("AnalyzerGain::process()"));

    SINT numFrames = count / m_channelCount;

    const CSAMPLE* pGainInput = pIn;
    CSAMPLE* pMixedChannel = nullptr;

    if (m_channelCount == mixxx::audio::ChannelCount::stem()) {
        // We have an 8 channel soundsource. The only implemented soundsource with
        // 8ch is the NI STEM file format.
        // TODO: If we add other soundsources with 8ch, we need to rework this condition.
        //
        // For NI STEM we mix all the stems together except the first one,
        // which contains drums or beats by convention.
        count = numFrames * mixxx::audio::ChannelCount::stereo();
        pMixedChannel = SampleUtil::alloc(count);
        VERIFY_OR_DEBUG_ASSERT(pMixedChannel) {
            return false;
        }
        SampleUtil::mixMultichannelToStereo(pMixedChannel, pIn, numFrames, m_channelCount);
        pGainInput = pMixedChannel;
    } else if (m_channelCount > mixxx::audio::ChannelCount::stereo()) {
        DEBUG_ASSERT(!"Unsupported channel count");
        return false;
    }

    if (numFrames > static_cast<SINT>(m_pLeftTempBuffer.size())) {
        m_pLeftTempBuffer.resize(numFrames);
        m_pRightTempBuffer.resize(numFrames);
    }
    SampleUtil::deinterleaveBuffer(m_pLeftTempBuffer.data(),
            m_pRightTempBuffer.data(),
            pGainInput,
            numFrames);
    SampleUtil::applyGain(m_pLeftTempBuffer.data(), 32767, numFrames);
    SampleUtil::applyGain(m_pRightTempBuffer.data(), 32767, numFrames);
    bool ret = m_pReplayGain->process(
            m_pLeftTempBuffer.data(), m_pRightTempBuffer.data(), numFrames);
    if (pMixedChannel) {
        SampleUtil::free(pMixedChannel);
    }
    return ret;
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
