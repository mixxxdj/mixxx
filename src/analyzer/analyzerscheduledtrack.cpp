#include "analyzer/analyzerscheduledtrack.h"

#include "analyzer/analyzertrack.h"
#include "track/trackid.h"

AnalyzerScheduledTrack::AnalyzerScheduledTrack(TrackId trackId, AnalyzerTrack::Options options)
        : trackId(trackId), options(options) {
}

const TrackId AnalyzerScheduledTrack::getTrackId() const {
    return trackId;
}

const AnalyzerTrack::Options AnalyzerScheduledTrack::getOptions() const {
    return options;
}
