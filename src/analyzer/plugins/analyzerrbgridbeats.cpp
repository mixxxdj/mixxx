#include "analyzer/plugins/analyzerrbgridbeats.h"

#include "analyzer/constants.h"
#include "analyzer/plugins/rbgrid/rbgridtracker.h"
#include "util/assert.h"

namespace mixxx {

AnalyzerRbGridBeats::AnalyzerRbGridBeats() = default;

AnalyzerRbGridBeats::~AnalyzerRbGridBeats() = default;

bool AnalyzerRbGridBeats::initialize(mixxx::audio::SampleRate sampleRate) {
    m_sampleRate = sampleRate;
    m_resultBpm = mixxx::Bpm();
    m_firstBeat = mixxx::audio::kStartFramePos;
    m_pTracker = std::make_unique<rbgrid::Tracker>(sampleRate.toDouble());
    return true;
}

bool AnalyzerRbGridBeats::processSamples(const CSAMPLE* pIn, SINT iLen) {
    VERIFY_OR_DEBUG_ASSERT(m_pTracker) {
        return false;
    }
    DEBUG_ASSERT(iLen % kAnalysisChannels == 0);
    m_pTracker->processStereo(pIn, static_cast<std::size_t>(iLen / kAnalysisChannels));
    return true;
}

bool AnalyzerRbGridBeats::finalize() {
    VERIFY_OR_DEBUG_ASSERT(m_pTracker) {
        return false;
    }
    const rbgrid::Grid grid = m_pTracker->finalize();
    m_pTracker.reset();
    if (!grid.valid) {
        return false;
    }
    m_resultBpm = mixxx::Bpm(grid.bpm);
    m_firstBeat = mixxx::audio::FramePos(grid.downbeatSec * m_sampleRate.toDouble());
    return true;
}

} // namespace mixxx
