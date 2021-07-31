#include "analyzer/plugins/analyzersoundtouchbeats.h"

#include <soundtouch/BPMDetect.h>

#include "analyzer/constants.h"
#include "util/sample.h"

namespace mixxx {

AnalyzerSoundTouchBeats::AnalyzerSoundTouchBeats()
        : m_downmixBuffer(kAnalysisFramesPerChunk) {
}

AnalyzerSoundTouchBeats::~AnalyzerSoundTouchBeats() {
}

bool AnalyzerSoundTouchBeats::initialize(mixxx::audio::SampleRate sampleRate) {
    m_resultBpm = mixxx::Bpm();
    m_pSoundTouch = std::make_unique<soundtouch::BPMDetect>(2, sampleRate);
    return true;
}

bool AnalyzerSoundTouchBeats::processSamples(const CSAMPLE* pIn, const int iLen) {
    if (!m_pSoundTouch) {
        return false;
    }
    DEBUG_ASSERT(iLen % kAnalysisChannels == 0);
    // We analyze a mono mixdown of the signal since we don't think stereo does
    // us any good.

    CSAMPLE* pDownmix = m_downmixBuffer.data();
    for (int i = 0; i < iLen / 2; i += 2) {
        pDownmix[i] = (pIn[i * 2] + pIn[i * 2 + 1]) * 0.5f;
    }

    m_pSoundTouch->inputSamples(pIn, iLen / 2);
    return true;
}

bool AnalyzerSoundTouchBeats::finalize() {
    if (!m_pSoundTouch) {
        return false;
    }
    m_resultBpm = mixxx::Bpm(m_pSoundTouch->getBpm());
    m_pSoundTouch.reset();
    return true;
}

} // namespace mixxx
