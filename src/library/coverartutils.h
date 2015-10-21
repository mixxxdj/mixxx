#ifndef COVERARTUTILS_H
#define COVERARTUTILS_H

#include <QImage>
#include <QString>
#include <QStringList>
#include <QSize>
#include <QFileInfo>
#include <QLinkedList>

#include "trackinfoobject.h"
#include "util/sandbox.h"

class CoverArtUtils {
  public:
    static QString defaultCoverLocation();

    // Extracts the first cover art image embedded within the file at
    // trackLocation. You must provide a security token for accessing
    // trackLocation.
    static QImage extractEmbeddedCover(
            const QString& trackLocation,
            SecurityTokenPointer pToken);

    static QImage loadCover(const CoverInfo& info);
    static quint16 calculateHash(const QImage& image) {
        return qChecksum(reinterpret_cast<const char*>(image.constBits()),
                         image.byteCount());
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

    // Guesses the cover art for the provided track. Does not modify the
    // provided track.
    static CoverArt guessCoverArt(
            TrackPointer pTrack);

    static QLinkedList<QFileInfo> findPossibleCoversInFolder(
            const QString& folder);

    // Selects an appropriate cover file from provided list of image files.
    static CoverArt selectCoverArtForTrack(
            TrackInfoObject* pTrack,
            const QLinkedList<QFileInfo>& covers);

    // Selects an appropriate cover file from provided list of image
    // files. Assumes a SecurityTokenPointer is held by the caller for all files
    // in 'covers'.
    static CoverArt selectCoverArtForTrack(
            const QString& trackBaseName,
            const QString& albumName,
            const QLinkedList<QFileInfo>& covers);


  private:
    CoverArtUtils() {}
};

#endif /* COVERARTUTILS_H */
