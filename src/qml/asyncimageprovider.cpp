#include "qml/asyncimageprovider.h"

#include <QThread>
#include <QtDebug>

#include "moc_asyncimageprovider.cpp"

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
}

QQuickTextureFactory* AsyncImageResponse::textureFactory() const {
    return QQuickTextureFactory::textureFactoryForImage(m_image);
}

void AsyncImageResponse::initialize() {
    if (!m_id.startsWith(kCoverArtPrefix)) {
        qWarning() << "ImageProvider: Unsupported ID" << m_id;
        emit finished();
        return;
    }
    const QString trackLocation =
            AsyncImageProvider::coverArtUrlIdToTrackLocation(m_id);

    // Lightweight: query CoverInfo directly from the database without
    // loading the full, stateful Track object.
    m_coverInfo = m_pTrackCollectionManager->getCoverInfoForTrackLocation(
            trackLocation);
    if (!m_coverInfo.hasImage()) {
        emit finished();
        return;
    }

    const int width = m_requestedSize.isValid() ? m_requestedSize.width() : 0;

    // Synchronous cache lookup. CoverArtCache uses QPixmapCache which
    // is only safe to access from the main thread, hence initialize()
    // is expected to run on the main thread.
    QPixmap pixmap = CoverArtCache::getCachedCover(m_coverInfo, width);
    if (!pixmap.isNull()) {
        m_image = pixmap.toImage();
        emit finished();
        return;
    }

    // Cache miss: request an asynchronous load from CoverArtCache.
    // The coverFound signal is emitted on the main thread when the
    // load completes. Since this response has been moved to the main
    // thread (see AsyncImageProvider::requestImageResponse), the
    // slot is invoked directly.
    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache) {
        connect(pCache,
                &CoverArtCache::coverFound,
                this,
                &AsyncImageResponse::slotCoverFound);
        CoverArtCache::requestUncachedCover(this, m_coverInfo, width);
    } else {
        qWarning() << "ImageProvider: CoverArtCache is not available";
        emit finished();
    }
}

void AsyncImageResponse::slotCoverFound(
        const QObject* pRequester,
        const CoverInfo& coverInfo,
        const QPixmap& pixmap) {
    if (pRequester != this) {
        return;
    }
    if (coverInfo.cacheKey() != m_coverInfo.cacheKey()) {
        return;
    }
    disconnect(CoverArtCache::instance(),
            &CoverArtCache::coverFound,
            this,
            &AsyncImageResponse::slotCoverFound);
    if (!pixmap.isNull()) {
        m_image = pixmap.toImage();
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
    auto* pResponse = new AsyncImageResponse(
            id, requestedSize, m_pTrackCollectionManager);
    // The response must live on the same thread as the
    // TrackCollectionManager (the main thread) because:
    //  - CoverArtCache::getCachedCover() accesses QPixmapCache which
    //    is not thread-safe.
    //  - CoverArtCache::coverFound is emitted on the main thread and
    //    the slot should be invoked there.
    // move the response before connecting any signals.
    pResponse->moveToThread(m_pTrackCollectionManager->thread());
    if (QThread::currentThread() != m_pTrackCollectionManager->thread()) {
        // requestImageResponse may be called from a non-main thread;
        // initialize() must run on the main thread.
        QMetaObject::invokeMethod(
                m_pTrackCollectionManager.get(),
                [pResponse] {
                    pResponse->initialize();
                },
                Qt::BlockingQueuedConnection);
    } else {
        pResponse->initialize();
    }
    return pResponse;
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
