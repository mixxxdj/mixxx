#include "track/trackinfo.h"


namespace mixxx {

void TrackInfo::resetUnsupportedValues() {
#if defined(__EXTRA_METADATA__)
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
#endif // __EXTRA_METADATA__
}

namespace {

const QString kArtistTitleSeparatorWithSpaces = " - ";
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
        fileName.replace(kArtistTitleSeparatorWithSpaces, kArtistTitleSeparator);
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
#if defined(__EXTRA_METADATA__)
            (lhs.getConductor() == rhs.getConductor()) &&
            (lhs.getDiscNumber() == rhs.getDiscNumber()) &&
            (lhs.getDiscTotal() == rhs.getDiscTotal()) &&
            (lhs.getEncoder() == rhs.getEncoder()) &&
            (lhs.getEncoderSettings() == rhs.getEncoderSettings()) &&
#endif // __EXTRA_METADATA__
            (lhs.getGenre() == rhs.getGenre()) &&
            (lhs.getGrouping() == rhs.getGrouping()) &&
#if defined(__EXTRA_METADATA__)
            (lhs.getISRC() == rhs.getISRC()) &&
#endif // __EXTRA_METADATA__
            (lhs.getKey() == rhs.getKey()) &&
#if defined(__EXTRA_METADATA__)
            (lhs.getLanguage() == rhs.getLanguage()) &&
            (lhs.getLyricist() == rhs.getLyricist()) &&
            (lhs.getMood() == rhs.getMood()) &&
            (lhs.getMovement() == rhs.getMovement()) &&
            (lhs.getMusicBrainzArtistId() == rhs.getMusicBrainzArtistId()) &&
            (lhs.getMusicBrainzRecordingId() == rhs.getMusicBrainzRecordingId()) &&
            (lhs.getMusicBrainzReleaseId() == rhs.getMusicBrainzReleaseId()) &&
            (lhs.getMusicBrainzWorkId() == rhs.getMusicBrainzWorkId()) &&
            (lhs.getRemixer() == rhs.getRemixer()) &&
#endif // __EXTRA_METADATA__
            (lhs.getReplayGain() == rhs.getReplayGain()) &&
#if defined(__EXTRA_METADATA__)
            (lhs.getSubtitle() == rhs.getSubtitle()) &&
#endif // __EXTRA_METADATA__
            (lhs.getTitle() == rhs.getTitle()) &&
            (lhs.getTrackNumber() == rhs.getTrackNumber()) &&
            (lhs.getTrackTotal() == rhs.getTrackTotal()) &&
#if defined(__EXTRA_METADATA__)
            (lhs.getWork() == rhs.getWork()) &&
#endif // __EXTRA_METADATA__
            (lhs.getYear() == rhs.getYear());
}

QDebug operator<<(QDebug dbg, const TrackInfo& arg) {
    dbg << '{';
    arg.dbgArtist(dbg);
    arg.dbgBpm(dbg);
    arg.dbgComment(dbg);
    arg.dbgComposer(dbg);
#if defined(__EXTRA_METADATA__)
    arg.dbgConductor(dbg);
    arg.dbgDiscNumber(dbg);
    arg.dbgDiscTotal(dbg);
    arg.dbgEncoder(dbg);
    arg.dbgEncoderSettings(dbg);
#endif // __EXTRA_METADATA__
    arg.dbgGenre(dbg);
    arg.dbgGrouping(dbg);
#if defined(__EXTRA_METADATA__)
    arg.dbgISRC(dbg);
#endif // __EXTRA_METADATA__
    arg.dbgKey(dbg);
#if defined(__EXTRA_METADATA__)
    arg.dbgLanguage(dbg);
    arg.dbgLyricist(dbg);
    arg.dbgMood(dbg);
    arg.dbgMovement(dbg);
    arg.dbgMusicBrainzArtistId(dbg);
    arg.dbgMusicBrainzRecordingId(dbg);
    arg.dbgMusicBrainzReleaseId(dbg);
    arg.dbgMusicBrainzWorkId(dbg);
    arg.dbgRemixer(dbg);
#endif // __EXTRA_METADATA__
    arg.dbgReplayGain(dbg);
#if defined(__EXTRA_METADATA__)
    arg.dbgSubtitle(dbg);
#endif // __EXTRA_METADATA__
    arg.dbgTitle(dbg);
    arg.dbgTrackNumber(dbg);
    arg.dbgTrackTotal(dbg);
#if defined(__EXTRA_METADATA__)
    arg.dbgWork(dbg);
#endif // __EXTRA_METADATA__
    arg.dbgYear(dbg);
    dbg << '}';
    return dbg;
}

} // namespace mixxx
