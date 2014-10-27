#ifndef COVERARTUTILS_H
#define COVERARTUTILS_H

#include <QByteArray>
#include <QImage>
#include <QString>
#include <QDir>
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

    static QString pixmapCacheKey(const QString& hash,
                                  const QSize& size) {
        if (size.isNull()) {
            return QString("CoverArtCache_%1").arg(hash);
        }
        return QString("CoverArtCache_%1_%2x%3")
                .arg(hash)
                .arg(size.width())
                .arg(size.height());
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

    static QString calculateHash(const QImage& image) {
        if (image.isNull()) {
            return QString();
        }
        return QString::number(
            qChecksum(reinterpret_cast<const char*>(image.constBits()),
                      image.byteCount()));
    }

    // Crops image to the provided size by first scaling to the appropriate
    // width and then cropping off the bottom. If size is taller than the image,
    // black pixels are padded on the bottom.
    static QImage cropImage(const QImage& image, const QSize& size) {
        if (image.isNull()) {
            return QImage();
        }
        QImage result = image.scaledToWidth(size.width(),
                                            Qt::SmoothTransformation);
        return result.copy(0, 0, image.width(), size.height());
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

    static QString supportedCoverArtExtensionsRegex() {
        QStringList extensions;
        extensions << "jpg" << "jpeg" << "png" << "gif" << "bmp";
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

    // Selects an appropriate cover file from provided list of image files.
    static CoverArt selectCoverArtForTrack(TrackInfoObject* pTrack,
                                           const QLinkedList<QFileInfo>& covers) {
        if (pTrack == NULL || covers.isEmpty()) {
            return CoverArt();
        }

        const QString trackBaseName = pTrack->getFileInfo().baseName();
        const QString albumName = pTrack->getAlbum();
        PreferredCoverType bestType = NONE;
        const QFileInfo* bestInfo = NULL;

        // TODO(XXX) Sort instead so that we can fall-back if one fails to open?
        int index = 0;
        for (QLinkedList<QFileInfo>::const_iterator it = covers.begin();
             it != covers.end(); ++it, ++index) {
            const QString coverBaseName = it->baseName();
            if (bestType > TRACK_BASENAME && coverBaseName == trackBaseName) {
                bestType = TRACK_BASENAME;
                bestInfo = &(*it);
                // This is the best type so we know we're done.
                break;
            } else if (bestType > ALBUM_NAME && coverBaseName == albumName) {
                bestType = ALBUM_NAME;
                bestInfo = &(*it);
            } else if (bestType > COVER && coverBaseName == "cover") {
                bestType = COVER;
                bestInfo = &(*it);
            } else if (bestType > FRONT && coverBaseName == "front") {
                bestType = FRONT;
                bestInfo = &(*it);
            } else if (bestType > ALBUM && coverBaseName == "album") {
                bestType = ALBUM;
                bestInfo = &(*it);
            } else if (bestType > FOLDER && coverBaseName == "folder") {
                bestType = FOLDER;
                bestInfo = &(*it);
            } else if (bestType > OTHER_FILENAME) {
                bestType = OTHER_FILENAME;
                bestInfo = &(*it);
            }
        }

        if (bestInfo != NULL) {
            CoverArt art;
            art.image = QImage(bestInfo->filePath());
            if (!art.image.isNull()) {
                art.info.source = CoverInfo::GUESSED;
                art.info.type = CoverInfo::FILE;
                art.info.hash = CoverArtUtils::calculateHash(art.image);
                art.info.coverLocation = bestInfo->fileName();
                return art;
            }
        }
        return CoverArt();
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
