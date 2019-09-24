#include "track/albuminfo.h"


namespace mixxx {

void AlbumInfo::resetUnsupportedValues() {
    setCopyright(QString());
    setLicense(QString());
    setMusicBrainzArtistId(QString());
    setMusicBrainzReleaseId(QString());
    setMusicBrainzReleaseGroupId(QString());
    setRecordLabel(QString());
    setReplayGain(ReplayGain());
}

bool operator==(const AlbumInfo& lhs, const AlbumInfo& rhs) {
    return (lhs.getArtist() == rhs.getArtist()) &&
            (lhs.getCopyright() == rhs.getCopyright()) &&
            (lhs.getLicense() == rhs.getLicense()) &&
            (lhs.getMusicBrainzArtistId() == rhs.getMusicBrainzArtistId()) &&
            (lhs.getMusicBrainzReleaseId() == rhs.getMusicBrainzReleaseId()) &&
            (lhs.getMusicBrainzReleaseGroupId() == rhs.getMusicBrainzReleaseGroupId()) &&
            (lhs.getRecordLabel() == rhs.getRecordLabel()) &&
            (lhs.getReplayGain() == rhs.getReplayGain()) &&
            (lhs.getTitle() == rhs.getTitle());
}

QDebug operator<<(QDebug dbg, const AlbumInfo& arg) {
    dbg << '{';
    arg.dbgArtist(dbg);
    arg.dbgCopyright(dbg);
    arg.dbgLicense(dbg);
    arg.dbgMusicBrainzArtistId(dbg);
    arg.dbgMusicBrainzReleaseId(dbg);
    arg.dbgMusicBrainzReleaseGroupId(dbg);
    arg.dbgRecordLabel(dbg);
    arg.dbgReplayGain(dbg);
    arg.dbgTitle(dbg);
    dbg << '}';
    return dbg;
}

} // namespace mixxx
