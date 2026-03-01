#pragma once

#include "engine/engine.h"

namespace mixxx {

// Analysis is done in blocks to avoid dynamic allocation of memory
// depending on the track length. A block size of 4096 frames per block
// seems to do fine. Signal processing during analysis uses the same,
// fixed number of channels like the engine does, usually 2 = stereo.
constexpr audio::ChannelCount kAnalysisChannels = mixxx::kEngineChannelOutputCount;
constexpr audio::ChannelCount kAnalysisMaxChannels = mixxx::kMaxEngineChannelInputCount;
constexpr SINT kAnalysisFramesPerChunk = 4096;
constexpr SINT kAnalysisSamplesPerChunk =
        kAnalysisFramesPerChunk * kAnalysisMaxChannels;

// Only analyze the first minute in fast-analysis mode.
constexpr SINT kFastAnalysisSecondsToAnalyze = 60;

}  // namespace mixxx
