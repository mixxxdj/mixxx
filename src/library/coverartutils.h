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
    // trackLocation.
    static QImage extractEmbeddedCover(const QString& trackLocation) {
        if (trackLocation.isEmpty()) {
            return QImage();
        }

        SecurityTokenPointer securityToken = Sandbox::openSecurityToken(
            QDir(trackLocation), true);
        SoundSourceProxy proxy(trackLocation, securityToken);
        Mixxx::SoundSource* pProxiedSoundSource = proxy.getProxiedSoundSource();
        if (pProxiedSoundSource == NULL) {
            return QImage();
        }
        return proxy.parseCoverArt();
    }

    static QImage loadCover(const CoverInfo& info) {
        if (info.type == CoverInfo::METADATA) {
            if (info.trackLocation.isEmpty()) {
                qDebug() << "CoverArtUtils::loadCover METADATA cover with empty trackLocation.";
                return QImage();
            }
            return CoverArtUtils::extractEmbeddedCover(info.trackLocation);
        } else if (info.type == CoverInfo::FILE) {
            if (info.trackLocation.isEmpty()) {
                qDebug() << "CoverArtUtils::loadCover FILE cover with empty trackLocation."
                         << "Relative paths will not work.";
                return QImage(info.coverLocation);
            }

            QFileInfo track(info.trackLocation);
            QFileInfo cover(track.dir(), info.coverLocation);

            if (!cover.exists()) {
                qDebug() << "CoverArtUtils::loadCover FILE cover does not exist:"
                         << info.coverLocation << info.trackLocation;
                return QImage();
            }
            return QImage(cover.filePath());
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
        OTHER_FILENAME,
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
        SecurityTokenPointer pToken = pTrack->getSecurityToken();
        SoundSourceProxy proxy(trackLocation, pToken);
        Mixxx::SoundSource* pProxiedSoundSource = proxy.getProxiedSoundSource();
        if (pProxiedSoundSource != NULL) {
            art.image = proxy.parseCoverArt();
            if (!art.image.isNull()) {
                art.info.hash = calculateHash(art.image);
                art.info.coverLocation = QString();
                art.info.type = CoverInfo::METADATA;
                qDebug() << "CoverArtUtils::guessCoverArt found metadata art" << art;
                return art;
            }
        }

        // Search for image files in the track directory.
        QRegExp coverArtFilenames(supportedCoverArtExtensionsRegex(),
                                  Qt::CaseInsensitive);
        QDirIterator it(trackInfo.absolutePath(),
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

        art = selectCoverArtForTrack(pTrack.data(), possibleCovers);
        if (art.info.type == CoverInfo::FILE) {
            qDebug() << "CoverArtUtils::guessCoverArt found file art" << art;
        } else {
            qDebug() << "CoverArtUtils::guessCoverArt didn't find art" << art;
        }
        return art;
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

    // Selects an appropriate cover file from provided list of image files.
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

        // TODO(XXX) Sort instead so that we can fall-back if one fails to open?
        foreach (const QFileInfo& file, covers) {
            const QString coverBaseName = file.baseName();
            if (bestType > TRACK_BASENAME && coverBaseName == trackBaseName) {
                bestType = TRACK_BASENAME;
                bestInfo = &file;
                // This is the best type so we know we're done.
                break;
            } else if (bestType > ALBUM_NAME && coverBaseName == albumName) {
                bestType = ALBUM_NAME;
                bestInfo = &file;
            } else if (bestType > COVER && coverBaseName == "cover") {
                bestType = COVER;
                bestInfo = &file;
            } else if (bestType > FRONT && coverBaseName == "front") {
                bestType = FRONT;
                bestInfo = &file;
            } else if (bestType > ALBUM && coverBaseName == "album") {
                bestType = ALBUM;
                bestInfo = &file;
            } else if (bestType > FOLDER && coverBaseName == "folder") {
                bestType = FOLDER;
                bestInfo = &file;
            } else if (bestType > OTHER_FILENAME) {
                bestType = OTHER_FILENAME;
                bestInfo = &file;
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

    static QString searchInTrackDirectory(const QString& directory,
                                          const QString& trackBaseName,
                                          const QString& album) {
        if (directory.isEmpty()) {
            return QString();
        }

        QDir dir(directory);
        dir.setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Readable);
        dir.setSorting(QDir::Size | QDir::Reversed);

        QStringList nameFilters;
        nameFilters << "*.jpg" << "*.jpeg" << "*.png" << "*.gif" << "*.bmp";
        dir.setNameFilters(nameFilters);
        QStringList imglist = dir.entryList();

        // no covers in this dir
        if (imglist.isEmpty()) {
            return QString();
            // only a single picture in folder.
        } else if (imglist.size() == 1) {
            return dir.filePath(imglist[0]);
        }

        QStringList prefNames;
        prefNames << trackBaseName  // cover with the same name of the trackFilename.
                  << album          // cover with the same name of the album.
                  << "cover"        // cover named as 'cover'
                  << "front"        // cover named as 'front'
                  << "album"        // cover named as 'album'
                  << "folder";      // cover named as 'folder'

        foreach (QString name, prefNames) {
            foreach (QString img, imglist) {
                if (img.contains(name, Qt::CaseInsensitive)) {
                    return dir.filePath(img);
                }
            }
        }

        // Return the lighter image file.
        return dir.filePath(imglist[0]);
    }


  private:
    CoverArtUtils() {}
};

#endif /* COVERARTUTILS_H */
