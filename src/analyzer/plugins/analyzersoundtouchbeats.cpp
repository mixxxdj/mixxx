#include "analyzer/plugins/analyzersoundtouchbeats.h"

#include <BPMDetect.h>

#include "util/sample.h"

namespace mixxx {

AnalyzerSoundTouchBeats::AnalyzerSoundTouchBeats() :
        m_downmixBuffer(4096) { // TODO(rryan) get this from somewhere
}

AnalyzerSoundTouchBeats::~AnalyzerSoundTouchBeats() { }

bool AnalyzerSoundTouchBeats::initialize(int samplerate) {
    m_fResultBpm = 0.0f;
    m_pSoundTouch = std::make_unique<soundtouch::BPMDetect>(2, samplerate);
    return true;
}

bool AnalyzerSoundTouchBeats::process(const CSAMPLE* pIn, const int iLen) {
    if (!m_pSoundTouch) {
        return false;
    }
    DEBUG_ASSERT(iLen == 4096 * 2); // TODO(rryan)
    DEBUG_ASSERT(iLen % 2 == 0);
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
    m_fResultBpm = m_pSoundTouch->getBpm();
    m_pSoundTouch.reset();
    return true;
}

}  // namespace mixxx
