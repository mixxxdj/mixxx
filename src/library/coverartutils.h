#pragma once

#include <QImage>
#include <QString>
#include <QStringList>
#include <QSize>
#include <QFileInfo>
#include <QList>

#include "track/track.h"
#include "util/sandbox.h"

class CoverInfo;
class CoverInfoRelative;


class CoverArtUtils {
  public:
    static QString defaultCoverLocation();

    // Extracts the first cover art image embedded within the file at
    // fileInfo. If no security token is provided a new one is created.
    static QImage extractEmbeddedCover(
            TrackFile trackFile);
    static QImage extractEmbeddedCover(
            TrackFile trackFile,
            SecurityTokenPointer pToken);

    static QImage loadCover(const CoverInfo& info);
    static quint16 calculateHash(const QImage& image) {
        return qChecksum(reinterpret_cast<const char*>(image.constBits()),
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
                         image.sizeInBytes()
#else
                         image.byteCount()
#endif
                );
    }

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

  private:
    QString m_cachedFolder;
    QList<QFileInfo> m_cachedPossibleCoversInFolder;
};
