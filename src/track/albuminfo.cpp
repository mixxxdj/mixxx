#include "track/albuminfo.h"


namespace mixxx {

bool operator==(const AlbumInfo& lhs, const AlbumInfo& rhs) {
    return (lhs.getArtist() == rhs.getArtist()) &&
            (lhs.getMusicBrainzArtistId() == rhs.getMusicBrainzArtistId()) &&
            (lhs.getMusicBrainzReleaseId() == rhs.getMusicBrainzReleaseId()) &&
            (lhs.getMusicBrainzReleaseGroupId() == rhs.getMusicBrainzReleaseGroupId()) &&
            (lhs.getReplayGain() == rhs.getReplayGain()) &&
            (lhs.getTitle() == rhs.getTitle());
}

} // namespace mixxx
