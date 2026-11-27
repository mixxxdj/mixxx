#pragma once

#include <QVector>
#include <optional>
#include <vector>

#include "analyzer/plugins/constantbeatgrid.h"
#include "audio/frame.h"
#include "audio/types.h"
#include "track/beatutils.h"

namespace mixxx {

// The result of makeQmBeatMap(): either a beat map or, for a track with a
// constant tempo, a constant grid.
struct QmBeatMap {
    QVector<audio::FramePos> beats;
    std::optional<ConstantBeatGrid> constantGrid;
};

// Builds a beat map from the beats of the QM tempo tracker, for tracks that
// may change tempo.
//
// 1. Regions: the beats are ironed into constant regions
//    (BeatUtils::retrieveConstRegions()), then regions with the same tempo
//    and phase are merged (qmbeatmap::mergeConstRegions()).
// 2. Extension: longest regions first, the grid of a region is extended over
//    its neighbours while the onsets agree with it: in every loud 16 beat
//    window the folded detection function peaks within 25 ms of a beat or
//    off-beat, and the kick drum (low band onsets) does not move to the other
//    half of the beat. The tracker loses the beat in breakdowns and drifts in
//    sparse parts, this replaces those parts by the grid of the music around
//    them.
// 3. Tempo: the tempo of each region of at least 32 beats is refined like
//    refineConstantBeatGrid() does over the whole track, keeping the centre
//    beat of the region.
// 4. Neighbours whose tempo differs by at most 0.3 % are merged if one line
//    fits all their beats within 30 ms, and two regions with nearly the same
//    tempo and phase are joined across up to 32 beats between them.
// 5. Half beat: the tracker often follows the off-beat (hi-hats, offbeat
//    bass). A logistic model compares the onsets and band levels on the beats
//    with those half way between them, on the beats of step 3. When the half
//    way positions look more like beats, every region is moved by half a
//    beat.
// 6. Boundaries: where two regions meet out of phase, the two beats on each
//    side are spaced evenly instead of leaving one short or long beat.
// 7. Constant tempo: when at least 75 % of the track is within 0.2 % of the
//    median tempo or of a simple ratio of it (2, 3/2, 4/3 and their inverses,
//    where the tracker follows another metric level such as triplets) and
//    constantGrid is set, the result is constantGrid. This is checked after
//    step 4, on the beat map before steps 5 and 6. The grid is moved by half a
//    beat if the half beat model of step 5 is confident and the grid lies
//    within 30 ms of the beats of the beat map while the model puts them on
//    the off-beats, or of its off-beats while the model puts them on the
//    beat.
//
// beats: tracker beats.
// onsets: detection function, one value per stepSizeFrames; beat positions
// map to onset frames as onsetutils::onsetFramePosition() does.
// bands: kQmOnsetBands magnitudes per onset frame, aligned with onsets.
// constantGrid: refineConstantBeatGrid() of the same track.
// Returns an empty beat map if there are fewer than two beats.
QmBeatMap makeQmBeatMap(
        const QVector<audio::FramePos>& beats,
        const std::vector<double>& onsets,
        const std::vector<float>& bands,
        audio::SampleRate sampleRate,
        int stepSizeFrames,
        const std::optional<ConstantBeatGrid>& constantGrid);

// Steps 1, 4 and 6 of makeQmBeatMap(), exposed for tests. Positions are in
// frames.
namespace qmbeatmap {

// A region of constant tempo. In a list of regions the last one only marks
// the end of the one before.
struct Region {
    double firstBeat;
    double beatLength;
};
using Regions = std::vector<Region>;

// Step 1: joins constant regions A and C (both >= 16 beats) and everything
// between them into one region, when A's grid extended over the gap still hits
// the first and the last beat of C and at least 40 % of the coarse beats in
// between (within 40 ms). The joined region is a least-squares fit of all
// coarse beats on that grid. Repeats until no join is possible.
// BeatUtils::retrieveConstRegions() splits a track at every tempo or phase
// wobble of the tracker; this undoes the splits that are not a real tempo
// change.
QVector<BeatUtils::ConstRegion> mergeConstRegions(
        const QVector<BeatUtils::ConstRegion>& constantRegions,
        const QVector<audio::FramePos>& coarseBeats,
        audio::SampleRate sampleRate);

// Merges neighbours as in step 4, then refines the tempo of the merged region.
Regions mergeNeighbours(Regions regions,
        const std::vector<double>& onsets,
        const std::vector<float>& bands,
        audio::SampleRate sampleRate,
        int stepSizeFrames);

// Joins regions across short regions between them as in step 4, then refines
// the tempo of the joined region.
Regions bridge(Regions regions,
        const std::vector<double>& onsets,
        const std::vector<float>& bands,
        audio::SampleRate sampleRate,
        int stepSizeFrames);

// Spaces the beats around too short or too long intervals evenly, step 6.
std::vector<double> smoothBoundaries(std::vector<double> positions);

} // namespace qmbeatmap

} // namespace mixxx
