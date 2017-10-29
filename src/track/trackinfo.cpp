#include "track/trackinfo.h"


namespace mixxx {

bool operator==(const TrackInfo& lhs, const TrackInfo& rhs) {
    return (lhs.getArtist() == rhs.getArtist()) &&
            (lhs.getBitrate() == rhs.getBitrate()) &&
            (lhs.getBpm() == rhs.getBpm()) &&
            (lhs.getChannels() == rhs.getChannels()) &&
            (lhs.getComment() == rhs.getComment()) &&
            (lhs.getComposer() == rhs.getComposer()) &&
            (lhs.getDuration() == rhs.getDuration()) &&
            (lhs.getGrouping() == rhs.getGrouping()) &&
            (lhs.getGenre() == rhs.getGenre()) &&
            (lhs.getKey() == rhs.getKey()) &&
            (lhs.getMusicBrainzArtistId() == rhs.getMusicBrainzArtistId()) &&
            (lhs.getMusicBrainzReleaseId() == rhs.getMusicBrainzReleaseId()) &&
            (lhs.getReplayGain() == rhs.getReplayGain()) &&
            (lhs.getSampleRate() == rhs.getSampleRate()) &&
            (lhs.getTitle() == rhs.getTitle()) &&
            (lhs.getTrackNumber() == rhs.getTrackNumber()) &&
            (lhs.getTrackTotal() == rhs.getTrackTotal()) &&
            (lhs.getYear() == rhs.getYear());
}

} // namespace mixxx
