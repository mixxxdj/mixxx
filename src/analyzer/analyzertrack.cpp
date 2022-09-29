#include "analyzer/analyzertrack.h"

AnalyzerTrack::AnalyzerTrack(TrackPointer track, Options options)
        : track(track), options(options) {
}

const TrackPointer AnalyzerTrack::getTrack() const {
    return track;
}

const AnalyzerTrack::Options AnalyzerTrack::getOptions() const {
    return options;
}
