#include "analyzer/analyzertrack.h"

#include "util/assert.h"

AnalyzerTrack::AnalyzerTrack(TrackPointer track, Options options)
        : track(track), options(options) {
    DEBUG_ASSERT(track);
}

const TrackPointer& AnalyzerTrack::getTrack() const {
    return track;
}

const AnalyzerTrack::Options AnalyzerTrack::getOptions() const {
    return options;
}
