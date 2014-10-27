#ifndef COVERARTUTILS_H
#define COVERARTUTILS_H

#include <QByteArray>
#include <QImage>
#include <QString>
#include <QDir>
#include <QStringList>
#include <QSize>

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
