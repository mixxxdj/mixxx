#ifndef COVERARTUTILS_H
#define COVERARTUTILS_H

#include <QImage>
#include <QString>
#include <QStringList>
#include <QSize>
#include <QFileInfo>
#include <QLinkedList>

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
    static CoverInfo guessCoverInfo(const Track& track);

    static QLinkedList<QFileInfo> findPossibleCoversInFolder(
            const QString& folder);

    // Selects an appropriate cover file from provided list of image files.
    static CoverInfo selectCoverArtForTrack(
            const Track& track,
            const QLinkedList<QFileInfo>& covers);

    // Selects an appropriate cover file from provided list of image
    // files. Assumes a SecurityTokenPointer is held by the caller for all files
    // in 'covers'.
    static CoverInfoRelative selectCoverArtForTrack(
            const TrackFile& trackFile,
            const QString& albumName,
            const QLinkedList<QFileInfo>& covers);


  private:
    CoverArtUtils() {}
};

#endif /* COVERARTUTILS_H */
