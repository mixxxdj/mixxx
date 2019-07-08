#pragma once

#include "engine/engine.h"
#include "util/audiosignal.h"

namespace mixxx {

// Analysis is done in blocks to avoid dynamic allocation of memory
// depending on the track length. A block size of 4096 frames per block
// seems to do fine. Signal processing during analysis uses the same,
// fixed number of channels like the engine does, usually 2 = stereo.
constexpr mixxx::AudioSignal::ChannelCount kAnalysisChannels = mixxx::kEngineChannelCount;
constexpr SINT kAnalysisFramesPerBlock = 4096;
constexpr SINT kAnalysisSamplesPerBlock =
        kAnalysisFramesPerBlock * kAnalysisChannels;

// Only analyze the first minute in fast-analysis mode.
constexpr int kFastAnalysisSecondsToAnalyze = 60;

}  // namespace mixxx
