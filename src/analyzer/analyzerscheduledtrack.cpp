#include "analyzer/analyzerscheduledtrack.h"

#include "analyzer/analyzertrack.h"
#include "track/trackid.h"

AnalyzerScheduledTrack::AnalyzerScheduledTrack(TrackId trackId, AnalyzerTrack::Options options)
        : m_trackId(trackId), m_options(options) {
}

const TrackId& AnalyzerScheduledTrack::getTrackId() const {
    return m_trackId;
}

const AnalyzerTrack::Options& AnalyzerScheduledTrack::getOptions() const {
    return m_options;
}
