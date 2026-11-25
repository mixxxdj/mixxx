#pragma once

#include <optional>
#include <vector>

#include "analyzer/plugins/constantbeatgrid.h"
#include "audio/types.h"

namespace mixxx {

// The half beat model of the QM beat analysis: the tracker often follows the
// off-beat (hi-hats, offbeat bass). A logistic model compares the onsets and
// band levels on the beats with those half way between them. Positions are in
// frames, onsets and bands as in makeQmBeatMap().

// Beyond this absolute score the model is confident: above it the beats are
// off-beats, below its negative they are on the beat. Otherwise the half beat
// check of refineConstantBeatGrid() decides the phase of a constant grid.
constexpr double kQmMinOffBeatScoreToMoveGrid = 0.5;

// The score of the half beat model, > 0 when the positions half way between
// the beats are the real beats.
double qmOffBeatScore(const std::vector<double>& beats,
        const std::vector<double>& onsets,
        const std::vector<float>& bands,
        int stepSizeFrames);

// Log ratio of the low band onset flux at positions to that at
// otherPositions, 0 if either has no onsets.
double qmLowBandFluxLogRatio(const std::vector<double>& positions,
        const std::vector<double>& otherPositions,
        const std::vector<double>& onsets,
        const std::vector<float>& bands,
        int stepSizeFrames);

// The constant grid of a track that is used without the beat map, moved by
// half a beat if the half beat model is confident that its beats are
// off-beats, as step 7 of makeQmBeatMap() moves the grid by the model's score
// on the beat map.
std::optional<ConstantBeatGrid> checkQmConstantGridHalfBeat(
        const std::optional<ConstantBeatGrid>& constantGrid,
        const std::vector<double>& onsets,
        const std::vector<float>& bands,
        audio::SampleRate sampleRate,
        int stepSizeFrames);

} // namespace mixxx
