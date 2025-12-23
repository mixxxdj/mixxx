#include "qml/asyncimageprovider.h"

#include "library/coverartcache.h"
#include "moc_asyncimageprovider.cpp"
#include "track/track.h"

namespace {
const QString kCoverArtPrefix = QStringLiteral("coverart/");
}

namespace mixxx {

namespace qml {

AsyncImageResponse::AsyncImageResponse(
        QString id,
        QSize requestedSize,
        std::shared_ptr<TrackCollectionManager> pTrackCollectionManager)
        : m_id(std::move(id)),
          m_requestedSize(requestedSize),
          m_pTrackCollectionManager(std::move(pTrackCollectionManager)) {
    setAutoDelete(false);
}

QQuickTextureFactory* AsyncImageResponse::textureFactory() const {
    return QQuickTextureFactory::textureFactoryForImage(m_image);
}

void AsyncImageResponse::run() {
    if (!m_id.startsWith(kCoverArtPrefix)) {
        qWarning() << "ImageProvider: Unsupported ID" << m_id;
        emit finished();
        return;
    }
    const QString trackLocation = AsyncImageProvider::coverArtUrlIdToTrackLocation(m_id);
    const auto trackRef = TrackRef::fromFilePath(trackLocation);

    // TODO: Only load CoverInfo from TrackCollectionManager
    // instead of the entire, stateful track object.
    TrackPointer pTrack;
    if (QThread::currentThread() != m_pTrackCollectionManager->thread()) {
        QMetaObject::invokeMethod(
                m_pTrackCollectionManager.get(),
                [this, trackRef] {
                    return m_pTrackCollectionManager->getTrackByRef(trackRef);
                },
                // This invocation will block the current thread!
                Qt::BlockingQueuedConnection,
                &pTrack);
    } else {
        pTrack = m_pTrackCollectionManager->getTrackByRef(trackRef);
    }
    if (!pTrack) {
        qWarning() << "ImageProvider: Failed to load track" << trackRef;
        emit finished();
        return;
    }
    const auto coverInfo = CoverInfo(pTrack->getCoverInfo(), trackLocation);
    // Release the track reference asap, i.e. before loading the image
    pTrack.reset();

    const CoverInfo::LoadedImage loadedImage = coverInfo.loadImage();
    switch (loadedImage.result) {
    case CoverInfo::LoadedImage::Result::NoImage:
        break;
    case CoverInfo::LoadedImage::Result::Ok:
        DEBUG_ASSERT(!loadedImage.image.isNull());
        if (m_requestedSize.isValid()) {
            m_image = loadedImage.image.scaled(m_requestedSize);
        } else {
            m_image = loadedImage.image;
        }
        break;
    default:
        qWarning() << "ImageProvider: Failed to load cover art" << trackRef;
        break;
    }

    emit finished();
}

AsyncImageProvider::AsyncImageProvider(
        std::shared_ptr<TrackCollectionManager> pTrackCollectionManager)
        : QQuickAsyncImageProvider(),
          m_pTrackCollectionManager(pTrackCollectionManager) {
}

QQuickImageResponse* AsyncImageProvider::requestImageResponse(
        const QString& id, const QSize& requestedSize) {
    AsyncImageResponse* response = new AsyncImageResponse(
            id, requestedSize, m_pTrackCollectionManager);
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
} // namespace mixxx
