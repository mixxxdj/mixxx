#ifndef COVERARTUTILS_H
#define COVERARTUTILS_H

#include <QByteArray>
#include <QImage>
#include <QString>
#include <QDir>
#include <QDirIterator>
#include <QStringList>
#include <QSize>
#include <QFileInfo>
#include <QLinkedList>

#include "util/sandbox.h"
#include "util/file.h"
#include "util/regex.h"
#include "soundsourceproxy.h"

class CoverArtUtils {
  public:
    static QString defaultCoverLocation() {
        return QString(":/images/library/default_cover.png");
    }

    static QString pixmapCacheKey(const quint16 hash,
                                  const int width) {
        if (width == 0) {
            return QString("CoverArtCache_%1").arg(QString::number(hash));
        }
        return QString("CoverArtCache_%1_%2")
                .arg(QString::number(hash)).arg(width);
    }

    // Extracts the first cover art image embedded within the file at
    // trackLocation. You must provide a security token for accessing
    // trackLocation.
    static QImage extractEmbeddedCover(const QString& trackLocation,
                                       SecurityTokenPointer pToken) {
        if (trackLocation.isEmpty()) {
            return QImage();
        }
        SoundSourceProxy proxy(trackLocation, pToken);
        Mixxx::SoundSourcePointer pSoundSource(proxy.getSoundSource());
        if (pSoundSource.isNull()) {
            return QImage();
        }
        return pSoundSource->parseCoverArt();
    }

    static QImage loadCover(const CoverInfo& info) {
        if (info.type == CoverInfo::METADATA) {
            if (info.trackLocation.isEmpty()) {
                qDebug() << "CoverArtUtils::loadCover METADATA cover with empty trackLocation.";
                return QImage();
            }
            SecurityTokenPointer pToken = Sandbox::openSecurityToken(
                QFileInfo(info.trackLocation), true);
            return CoverArtUtils::extractEmbeddedCover(info.trackLocation,
                                                       pToken);
        } else if (info.type == CoverInfo::FILE) {
            if (info.trackLocation.isEmpty()) {
                qDebug() << "CoverArtUtils::loadCover FILE cover with empty trackLocation."
                         << "Relative paths will not work.";
                SecurityTokenPointer pToken = Sandbox::openSecurityToken(
                    QFileInfo(info.coverLocation), true);
                return QImage(info.coverLocation);
            }

            QFileInfo track(info.trackLocation);
            QFileInfo cover(track.dir(), info.coverLocation);

            if (!cover.exists()) {
                qDebug() << "CoverArtUtils::loadCover FILE cover does not exist:"
                         << info.coverLocation << info.trackLocation;
                return QImage();
            }
            QString coverPath = cover.filePath();
            SecurityTokenPointer pToken = Sandbox::openSecurityToken(
                cover, true);
            return QImage(coverPath);
        } else {
            qDebug() << "CoverArtUtils::loadCover bad type";
            return QImage();
        }
    }

    static quint16 calculateHash(const QImage& image) {
        return qChecksum(reinterpret_cast<const char*>(image.constBits()),
                         image.byteCount());
    }

    // Resizes the image (preserving aspect ratio) if it is larger than
    // maxEdgeSize on either side.
    static QImage maybeResizeImage(const QImage& image, int maxEdgeSize) {
        if (image.width() > maxEdgeSize || image.height() > maxEdgeSize) {
            return image.scaled(maxEdgeSize, maxEdgeSize, Qt::KeepAspectRatio,
                                Qt::SmoothTransformation);
        }
        return image;
    }

    // Resizes the image (preserving aspect ratio) to width.
    static QImage resizeImage(const QImage& image, int width) {
        return image.scaledToWidth(width, Qt::SmoothTransformation);
    }

    static QStringList supportedCoverArtExtensions() {
        QStringList extensions;
        extensions << "jpg" << "jpeg" << "png" << "gif" << "bmp";
        return extensions;
    }

    static QString supportedCoverArtExtensionsRegex() {
        QStringList extensions = supportedCoverArtExtensions();
        return RegexUtils::fileExtensionsRegex(extensions);
    }

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
    static CoverArt guessCoverArt(TrackPointer pTrack) {
        CoverArt art;
        art.info.source = CoverInfo::GUESSED;

        if (pTrack.isNull()) {
            return art;
        }

        const QFileInfo trackInfo = pTrack->getFileInfo();
        const QString trackLocation = trackInfo.absoluteFilePath();

        art.image = extractEmbeddedCover(trackLocation, pTrack->getSecurityToken());
        if (!art.image.isNull()) {
            art.info.hash = calculateHash(art.image);
            art.info.coverLocation = QString();
            art.info.type = CoverInfo::METADATA;
            qDebug() << "CoverArtUtils::guessCoverArt found metadata art" << art;
            return art;
        }

        QLinkedList<QFileInfo> possibleCovers = findPossibleCoversInFolder(
            trackInfo.absolutePath());
        art = selectCoverArtForTrack(pTrack.data(), possibleCovers);
        if (art.info.type == CoverInfo::FILE) {
            qDebug() << "CoverArtUtils::guessCoverArt found file art" << art;
        } else {
            qDebug() << "CoverArtUtils::guessCoverArt didn't find art" << art;
        }
        return art;
    }

    static QLinkedList<QFileInfo> findPossibleCoversInFolder(const QString& folder) {
        // Search for image files in the track directory.
        QRegExp coverArtFilenames(supportedCoverArtExtensionsRegex(),
                                  Qt::CaseInsensitive);
        QDirIterator it(folder,
                        QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
        QFile currentFile;
        QFileInfo currentFileInfo;
        QLinkedList<QFileInfo> possibleCovers;
        while (it.hasNext()) {
            it.next();
            currentFileInfo = it.fileInfo();
            if (currentFileInfo.isFile() &&
                coverArtFilenames.indexIn(currentFileInfo.fileName()) != -1) {
                possibleCovers.append(currentFileInfo);
            }
        }
        return possibleCovers;
    }

    // Selects an appropriate cover file from provided list of image files.
    static CoverArt selectCoverArtForTrack(TrackInfoObject* pTrack,
                                           const QLinkedList<QFileInfo>& covers) {
        if (pTrack == NULL || covers.isEmpty()) {
            CoverArt art;
            art.info.source = CoverInfo::GUESSED;
            return art;
        }

        const QString trackBaseName = pTrack->getFileInfo().baseName();
        const QString albumName = pTrack->getAlbum();
        return selectCoverArtForTrack(trackBaseName, albumName, covers);
    }

    // Selects an appropriate cover file from provided list of image
    // files. Assumes a SecurityTokenPointer is held by the caller for all files
    // in 'covers'.
    static CoverArt selectCoverArtForTrack(const QString& trackBaseName,
                                           const QString& albumName,
                                           const QLinkedList<QFileInfo>& covers) {
        CoverArt art;
        art.info.source = CoverInfo::GUESSED;
        if (covers.isEmpty()) {
            return art;
        }

        PreferredCoverType bestType = NONE;
        const QFileInfo* bestInfo = NULL;

        // If there is a single image then we use it unconditionally. Otherwise
        // we use the priority order described in PreferredCoverType. Notably,
        // if there are multiple image files in the folder we require they match
        // one of the name patterns -- otherwise we run the risk of picking an
        // arbitrary image that happens to be in the same folder as some of the
        // user's music files.
        if (covers.size() == 1) {
            bestInfo = &covers.first();
        } else {
            // TODO(XXX) Sort instead so that we can fall-back if one fails to
            // open?
            foreach (const QFileInfo& file, covers) {
                const QString coverBaseName = file.baseName();
                if (bestType > TRACK_BASENAME &&
                    coverBaseName.compare(trackBaseName,
                                          Qt::CaseInsensitive) == 0) {
                    bestType = TRACK_BASENAME;
                    bestInfo = &file;
                    // This is the best type so we know we're done.
                    break;
                } else if (bestType > ALBUM_NAME &&
                           coverBaseName.compare(albumName,
                                                 Qt::CaseInsensitive) == 0) {
                    bestType = ALBUM_NAME;
                    bestInfo = &file;
                } else if (bestType > COVER &&
                           coverBaseName.compare(QLatin1String("cover"),
                                                 Qt::CaseInsensitive) == 0) {
                    bestType = COVER;
                    bestInfo = &file;
                } else if (bestType > FRONT &&
                           coverBaseName.compare(QLatin1String("front"),
                                                 Qt::CaseInsensitive) == 0) {
                    bestType = FRONT;
                    bestInfo = &file;
                } else if (bestType > ALBUM &&
                           coverBaseName.compare(QLatin1String("album"),
                                                 Qt::CaseInsensitive) == 0) {
                    bestType = ALBUM;
                    bestInfo = &file;
                } else if (bestType > FOLDER &&
                           coverBaseName.compare(QLatin1String("folder"),
                                                 Qt::CaseInsensitive) == 0) {
                    bestType = FOLDER;
                    bestInfo = &file;
                }
            }
        }

        if (bestInfo != NULL) {
            art.image = QImage(bestInfo->filePath());
            if (!art.image.isNull()) {
                art.info.source = CoverInfo::GUESSED;
                art.info.type = CoverInfo::FILE;
                art.info.hash = CoverArtUtils::calculateHash(art.image);
                art.info.coverLocation = bestInfo->fileName();
                return art;
            }
        }

        return art;
    }


  private:
    CoverArtUtils() {}
};

#endif /* COVERARTUTILS_H */
