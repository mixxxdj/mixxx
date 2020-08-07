#include "track/albuminfo.h"


namespace mixxx {

bool operator==(const AlbumInfo& lhs, const AlbumInfo& rhs) {
    return (lhs.getArtist() == rhs.getArtist()) &&
#if defined(__EXTRA_METADATA__)
            (lhs.getCopyright() == rhs.getCopyright()) &&
            (lhs.getLicense() == rhs.getLicense()) &&
            (lhs.getMusicBrainzArtistId() == rhs.getMusicBrainzArtistId()) &&
            (lhs.getMusicBrainzReleaseGroupId() == rhs.getMusicBrainzReleaseGroupId()) &&
            (lhs.getMusicBrainzReleaseId() == rhs.getMusicBrainzReleaseId()) &&
            (lhs.getRecordLabel() == rhs.getRecordLabel()) &&
            (lhs.getReplayGain() == rhs.getReplayGain()) &&
#endif // __EXTRA_METADATA__
            (lhs.getTitle() == rhs.getTitle());
}

QDebug operator<<(QDebug dbg, const AlbumInfo& arg) {
    dbg << "AlbumInfo{";
    arg.dbgArtist(dbg);
#if defined(__EXTRA_METADATA__)
    arg.dbgCopyright(dbg);
    arg.dbgLicense(dbg);
    arg.dbgMusicBrainzArtistId(dbg);
    arg.dbgMusicBrainzReleaseId(dbg);
    arg.dbgMusicBrainzReleaseGroupId(dbg);
    arg.dbgRecordLabel(dbg);
    arg.dbgReplayGain(dbg);
#endif // __EXTRA_METADATA__
    arg.dbgTitle(dbg);
    dbg << '}';
    return dbg;
}

} // namespace mixxx
