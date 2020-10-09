#pragma once

#include <QFileInfo>
#include <QImage>
#include <QList>
#include <QSize>
#include <QString>
#include <QStringList>

#include "track/track.h"
#include "util/sandbox.h"

class CoverInfo;
class CoverInfoRelative;
class TrackFile;

class CoverArtUtils {
  public:
    static QString defaultCoverLocation();

    // Extracts the first cover art image embedded within the file.
    static QImage extractEmbeddedCover(
            TrackFile trackFile,
            SecurityTokenPointer pToken);

    static QStringList supportedCoverArtExtensions();
    static QString supportedCoverArtExtensionsRegex();

    enum PreferredCoverType {
        TRACK_BASENAME = 0,
        ALBUM_NAME,
        COVER,
        FRONT,
        ALBUM,
        FOLDER,
        NONE
    };

    // Guesses the cover art for the provided track.
    static CoverInfoRelative guessCoverInfo(
            const Track& track);

    static QList<QFileInfo> findPossibleCoversInFolder(
            const QString& folder);

    // Selects an appropriate cover file from provided list of image files.
    static CoverInfoRelative selectCoverArtForTrack(
            const Track& track,
            const QList<QFileInfo>& covers);

    // Selects an appropriate cover file from provided list of image
    // files. Assumes a SecurityTokenPointer is held by the caller for all files
    // in 'covers'.
    static CoverInfoRelative selectCoverArtForTrack(
            const TrackFile& trackFile,
            const QString& albumName,
            const QList<QFileInfo>& covers);


  private:
    CoverArtUtils() {}
};

// Stateful guessing of cover art by caching the possible
// covers from the last visited folder.
class CoverInfoGuesser {
  public:
    // Guesses the cover art for the provided track.
    // An embedded cover must be extracted beforehand and provided.
    CoverInfoRelative guessCoverInfo(
            const TrackFile& trackFile,
            const QString& albumName,
            const QImage& embeddedCover);

    // Extracts an embedded cover image if available and guesses
    // the cover art for the provided track.
    CoverInfoRelative guessCoverInfoForTrack(
            const Track& track);

    void guessAndSetCoverInfoForTrack(
            Track& track) {
        track.setCoverInfo(guessCoverInfoForTrack(track));
    }
    void guessAndSetCoverInfoForTracks(
            const QList<TrackPointer>& tracks);

  private:
    QString m_cachedFolder;
    QList<QFileInfo> m_cachedPossibleCoversInFolder;
};

// Guesses the cover art for the provided tracks by searching the tracks'
// metadata and folders for image files. All I/O is done in a separate
// thread.
void guessTrackCoverInfoConcurrently(TrackPointer pTrack);

// Concurrent guessing of track covers during short running
// tests may cause spurious test failures due to timing issues.
void disableConcurrentGuessingOfTrackCoverInfoDuringTests();
