#include "track/albuminfo.h"


namespace mixxx {

bool operator==(const AlbumInfo& lhs, const AlbumInfo& rhs) {
    return (lhs.getArtist() == rhs.getArtist()) &&
            (lhs.getMusicBrainzId() == rhs.getMusicBrainzId()) &&
            (lhs.getReplayGain() == rhs.getReplayGain()) &&
            (lhs.getTitle() == rhs.getTitle());
}

} // namespace mixxx
