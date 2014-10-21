#ifndef COVERARTUTILS_H
#define COVERARTUTILS_H

#include <QByteArray>
#include <QImage>
#include <QString>
#include <QDir>

#include "util/sandbox.h"
#include "soundsourceproxy.h"

class CoverArtUtils {
  public:
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

  private:
    CoverArtUtils() {}
};

#endif /* COVERARTUTILS_H */
