#include "track/trackinfo.h"


namespace mixxx {

void TrackInfo::resetUnsupportedValues() {
    setConductor(QString());
    setDiscNumber(QString());
    setDiscTotal(QString());
    setEncoder(QString());
    setEncoderSettings(QString());
    setISRC(QString());
    setLanguage(QString());
    setLyricist(QString());
    setMood(QString());
    setMovement(QString());
    setMusicBrainzArtistId(QString());
    setMusicBrainzRecordingId(QString());
    setMusicBrainzReleaseId(QString());
    setMusicBrainzWorkId(QString());
    setRemixer(QString());
    setSubtitle(QString());
    setWork(QString());
}

namespace {

const QString kArtistTitleSeparator = "_-_";

const QChar kFileExtensionSeparator = '.';

} // anonymous namespace

bool TrackInfo::parseArtistTitleFromFileName(
        QString fileName,
        bool splitArtistTitle) {
    bool modified = false;
    fileName = fileName.trimmed();
    auto titleWithFileType = fileName;
    if (splitArtistTitle) {
        fileName.replace(" - ", kArtistTitleSeparator);
        if (fileName.count(kArtistTitleSeparator) == 1) {
            auto artist = fileName.section(kArtistTitleSeparator, 0, 0).trimmed();
            if (!artist.isEmpty()) {
                setArtist(artist);
                modified = true;
            }
            titleWithFileType = fileName.section(kArtistTitleSeparator, 1).trimmed();
        }
    }
    auto title = titleWithFileType;
    if (titleWithFileType.contains(kFileExtensionSeparator)) {
        // Strip file extension starting at the right-most '.'
        title = titleWithFileType.section(kFileExtensionSeparator, 0, -2);
    }
    title = title.trimmed();
    if (!title.isEmpty()) {
        setTitle(title);
        modified = true;
    }
    return modified;
}

bool operator==(const TrackInfo& lhs, const TrackInfo& rhs) {
    return (lhs.getArtist() == rhs.getArtist()) &&
            (lhs.getBpm() == rhs.getBpm()) &&
            (lhs.getComment() == rhs.getComment()) &&
            (lhs.getComposer() == rhs.getComposer()) &&
            (lhs.getConductor() == rhs.getConductor()) &&
            (lhs.getDiscNumber() == rhs.getDiscNumber()) &&
            (lhs.getDiscTotal() == rhs.getDiscTotal()) &&
            (lhs.getEncoder() == rhs.getEncoder()) &&
            (lhs.getEncoderSettings() == rhs.getEncoderSettings()) &&
            (lhs.getGenre() == rhs.getGenre()) &&
            (lhs.getGrouping() == rhs.getGrouping()) &&
            (lhs.getISRC() == rhs.getISRC()) &&
            (lhs.getKey() == rhs.getKey()) &&
            (lhs.getLanguage() == rhs.getLanguage()) &&
            (lhs.getLyricist() == rhs.getLyricist()) &&
            (lhs.getMood() == rhs.getMood()) &&
            (lhs.getMovement() == rhs.getMovement()) &&
            (lhs.getMusicBrainzArtistId() == rhs.getMusicBrainzArtistId()) &&
            (lhs.getMusicBrainzRecordingId() == rhs.getMusicBrainzRecordingId()) &&
            (lhs.getMusicBrainzReleaseId() == rhs.getMusicBrainzReleaseId()) &&
            (lhs.getMusicBrainzWorkId() == rhs.getMusicBrainzWorkId()) &&
            (lhs.getRemixer() == rhs.getRemixer()) &&
            (lhs.getReplayGain() == rhs.getReplayGain()) &&
            (lhs.getSubtitle() == rhs.getSubtitle()) &&
            (lhs.getTitle() == rhs.getTitle()) &&
            (lhs.getTrackNumber() == rhs.getTrackNumber()) &&
            (lhs.getTrackTotal() == rhs.getTrackTotal()) &&
            (lhs.getWork() == rhs.getWork()) &&
            (lhs.getYear() == rhs.getYear());
}

QDebug operator<<(QDebug dbg, const TrackInfo& arg) {
    dbg << '{';
    arg.dbgArtist(dbg);
    arg.dbgBpm(dbg);
    arg.dbgComment(dbg);
    arg.dbgComposer(dbg);
    arg.dbgConductor(dbg);
    arg.dbgDiscNumber(dbg);
    arg.dbgDiscTotal(dbg);
    arg.dbgEncoder(dbg);
    arg.dbgEncoderSettings(dbg);
    arg.dbgGenre(dbg);
    arg.dbgGrouping(dbg);
    arg.dbgISRC(dbg);
    arg.dbgKey(dbg);
    arg.dbgLanguage(dbg);
    arg.dbgLyricist(dbg);
    arg.dbgMood(dbg);
    arg.dbgMovement(dbg);
    arg.dbgMusicBrainzArtistId(dbg);
    arg.dbgMusicBrainzRecordingId(dbg);
    arg.dbgMusicBrainzReleaseId(dbg);
    arg.dbgMusicBrainzWorkId(dbg);
    arg.dbgRemixer(dbg);
    arg.dbgReplayGain(dbg);
    arg.dbgSubtitle(dbg);
    arg.dbgTitle(dbg);
    arg.dbgTrackNumber(dbg);
    arg.dbgTrackTotal(dbg);
    arg.dbgWork(dbg);
    arg.dbgYear(dbg);
    dbg << '}';
    return dbg;
}

} // namespace mixxx
