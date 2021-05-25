#include "skin/qml/asyncimageprovider.h"

#include "library/coverartcache.h"

namespace mixxx {
namespace skin {
namespace qml {

AsyncImageResponse::AsyncImageResponse(const QString& id, const QSize& requestedSize)
        : m_id(id), m_requestedSize(requestedSize) {
    setAutoDelete(false);
}

QQuickTextureFactory* AsyncImageResponse::textureFactory() const {
    return QQuickTextureFactory::textureFactoryForImage(m_image);
}

void AsyncImageResponse::run() {
    if (m_id.startsWith(QStringLiteral("coverart/"))) {
        QString trackLocation = QString::fromUtf8(QByteArray::fromBase64(
                m_id.mid(9).toLatin1(), QByteArray::Base64UrlEncoding));

        // TODO: This code does not allow to override embedded cover art with
        // a custom image, which is possible in Mixxx. We need to access the
        // actual CoverInfo of the track instead of constructing a default
        // instance on the fly.
        CoverInfo coverInfo(CoverInfoRelative(), trackLocation);
        coverInfo.type = CoverInfoRelative::METADATA;
        CoverInfo::LoadedImage loadedImage = coverInfo.loadImage();
        if (loadedImage.result != CoverInfo::LoadedImage::Result::Ok) {
            coverInfo.type = CoverInfoRelative::FILE;
            loadedImage = coverInfo.loadImage();
        }
        m_image = loadedImage.image;
    } else {
        qWarning() << "ImageProvider: Unknown ID " << m_id;
    }

    if (!m_image.isNull() && m_requestedSize.isValid()) {
        m_image = m_image.scaled(m_requestedSize);
    }

    emit finished();
}

QQuickImageResponse* AsyncImageProvider::requestImageResponse(
        const QString& id, const QSize& requestedSize) {
    AsyncImageResponse* response = new AsyncImageResponse(id, requestedSize);
    pool.start(response);
    return response;
}

} // namespace qml
} // namespace skin
} // namespace mixxx
