#include "track/trackinfo.h"


namespace mixxx {

void TrackInfo::resetUnsupportedValues() {
    setConductor(QString());
    setISRC(QString());
    setLanguage(QString());
    setLyricist(QString());
    setMood(QString());
    setMusicBrainzArtistId(QString());
    setMusicBrainzReleaseId(QString());
    setRemixer(QString());
    setSubtitle(QString());
}

bool operator==(const TrackInfo& lhs, const TrackInfo& rhs) {
    return (lhs.getArtist() == rhs.getArtist()) &&
            (lhs.getBpm() == rhs.getBpm()) &&
            (lhs.getComment() == rhs.getComment()) &&
            (lhs.getComposer() == rhs.getComposer()) &&
            (lhs.getConductor() == rhs.getConductor()) &&
            (lhs.getGenre() == rhs.getGenre()) &&
            (lhs.getGrouping() == rhs.getGrouping()) &&
            (lhs.getISRC() == rhs.getISRC()) &&
            (lhs.getKey() == rhs.getKey()) &&
            (lhs.getLanguage() == rhs.getLanguage()) &&
            (lhs.getLyricist() == rhs.getLyricist()) &&
            (lhs.getMood() == rhs.getMood()) &&
            (lhs.getMusicBrainzArtistId() == rhs.getMusicBrainzArtistId()) &&
            (lhs.getMusicBrainzReleaseId() == rhs.getMusicBrainzReleaseId()) &&
            (lhs.getRemixer() == rhs.getRemixer()) &&
            (lhs.getReplayGain() == rhs.getReplayGain()) &&
            (lhs.getSubtitle() == rhs.getSubtitle()) &&
            (lhs.getTitle() == rhs.getTitle()) &&
            (lhs.getTrackNumber() == rhs.getTrackNumber()) &&
            (lhs.getTrackTotal() == rhs.getTrackTotal()) &&
            (lhs.getYear() == rhs.getYear());
}

QDebug operator<<(QDebug dbg, const TrackInfo& arg) {
    dbg << '{';
    arg.dbgArtist(dbg);
    arg.dbgBpm(dbg);
    arg.dbgComment(dbg);
    arg.dbgComposer(dbg);
    arg.dbgConductor(dbg);
    arg.dbgGenre(dbg);
    arg.dbgGrouping(dbg);
    arg.dbgISRC(dbg);
    arg.dbgKey(dbg);
    arg.dbgLanguage(dbg);
    arg.dbgLyricist(dbg);
    arg.dbgMood(dbg);
    arg.dbgMusicBrainzArtistId(dbg);
    arg.dbgMusicBrainzReleaseId(dbg);
    arg.dbgRemixer(dbg);
    arg.dbgReplayGain(dbg);
    arg.dbgSubtitle(dbg);
    arg.dbgTitle(dbg);
    arg.dbgTrackNumber(dbg);
    arg.dbgTrackTotal(dbg);
    arg.dbgYear(dbg);
    dbg << '}';
    return dbg;
}

} // namespace mixxx
