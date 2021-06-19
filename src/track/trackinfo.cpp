#include "track/trackinfo.h"


namespace mixxx {

namespace {

const QString kArtistTitleSeparatorWithSpaces = QStringLiteral(" - ");
const QString kArtistTitleSeparator = QStringLiteral("_-_");

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

bool TrackInfo::compareEq(
        const TrackInfo& trackInfo,
        Bpm::Comparison cmpBpm) const {
    return (getArtist() == trackInfo.getArtist()) &&
            getBpm().compareEq(trackInfo.getBpm(), cmpBpm) &&
            (getComment() == trackInfo.getComment()) &&
            (getComposer() == trackInfo.getComposer()) &&
#if defined(__EXTRA_METADATA__)
            (getConductor() == trackInfo.getConductor()) &&
            (getDiscNumber() == trackInfo.getDiscNumber()) &&
            (getDiscTotal() == trackInfo.getDiscTotal()) &&
            (getEncoder() == trackInfo.getEncoder()) &&
            (getEncoderSettings() == trackInfo.getEncoderSettings()) &&
#endif // __EXTRA_METADATA__
            (getGenre() == trackInfo.getGenre()) &&
            (getGrouping() == trackInfo.getGrouping()) &&
#if defined(__EXTRA_METADATA__)
            (getISRC() == trackInfo.getISRC()) &&
#endif // __EXTRA_METADATA__
            (getKey() == trackInfo.getKey()) &&
#if defined(__EXTRA_METADATA__)
            (getLanguage() == trackInfo.getLanguage()) &&
            (getLyricist() == trackInfo.getLyricist()) &&
            (getMood() == trackInfo.getMood()) &&
            (getMovement() == trackInfo.getMovement()) &&
            (getMusicBrainzArtistId() == trackInfo.getMusicBrainzArtistId()) &&
            (getMusicBrainzRecordingId() == trackInfo.getMusicBrainzRecordingId()) &&
            (getMusicBrainzReleaseId() == trackInfo.getMusicBrainzReleaseId()) &&
            (getMusicBrainzWorkId() == trackInfo.getMusicBrainzWorkId()) &&
            (getRemixer() == trackInfo.getRemixer()) &&
#endif // __EXTRA_METADATA__
            (getReplayGain() == trackInfo.getReplayGain()) &&
            (getSeratoTags() == trackInfo.getSeratoTags()) &&
#if defined(__EXTRA_METADATA__)
            (getSubtitle() == trackInfo.getSubtitle()) &&
#endif // __EXTRA_METADATA__
            (getTitle() == trackInfo.getTitle()) &&
            (getTrackNumber() == trackInfo.getTrackNumber()) &&
            (getTrackTotal() == trackInfo.getTrackTotal()) &&
#if defined(__EXTRA_METADATA__)
            (getWork() == trackInfo.getWork()) &&
#endif // __EXTRA_METADATA__
            (getYear() == trackInfo.getYear());
}

QDebug operator<<(QDebug dbg, const TrackInfo& arg) {
    dbg << "TrackInfo{";
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
    arg.dbgSeratoTags(dbg);
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
