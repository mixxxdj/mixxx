#include "skin/qml/asyncimageprovider.h"

#include "library/coverartcache.h"

namespace {
const QString kCoverArtPrefix = QStringLiteral("coverart/");
}

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
    if (m_id.startsWith(kCoverArtPrefix)) {
        QString trackLocation = AsyncImageProvider::coverArtUrlIdToTrackLocation(m_id);

        // TODO: This code does not allow to override embedded cover art with
        // a custom image, which is possible in Mixxx. We need to access the
        // actual CoverInfo of the track instead of constructing a default
        // instance on the fly.
        //
        // Unfortunately, TrackCollectionManager::getTrackByRef will not work
        // when called from another thread (like this one). We need a solution
        // for that.
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

// static
const QString AsyncImageProvider::kProviderName = QStringLiteral("mixxx");

// static
QUrl AsyncImageProvider::trackLocationToCoverArtUrl(const QString& trackLocation) {
    QUrl url("image://" + kProviderName + "/" + kCoverArtPrefix);
    return url.resolved(
            QString::fromLatin1(trackLocation.toUtf8().toBase64(
                    QByteArray::Base64UrlEncoding)));
}

//static
QString AsyncImageProvider::coverArtUrlIdToTrackLocation(const QString& coverArtUrlId) {
    return QString::fromUtf8(QByteArray::fromBase64(
            coverArtUrlId.mid(kCoverArtPrefix.size()).toLatin1(), QByteArray::Base64UrlEncoding));
}

} // namespace qml
} // namespace skin
} // namespace mixxx
