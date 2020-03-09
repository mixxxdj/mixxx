#include <QFutureWatcher>
#include <QPixmapCache>
#include <QtConcurrentRun>
#include <QtDebug>

#include "library/coverartcache.h"
#include "library/coverartutils.h"
#include "util/compatibility.h"
#include "util/logger.h"


namespace {

mixxx::Logger kLogger("CoverArtCache");

// The initial QPixmapCache limit is 10MB.
// But it is not used just by the coverArt stuff,
// it is also used by Qt to handle other things behind the scenes.
// Consequently coverArt cache will always have less than those
// 10MB available to store the pixmaps.
// So, we must increase this size a bit more,
// in order to allow CoverCache handle more covers (performance gain).
constexpr int kPixmapCacheLimit = 20480;

QString pixmapCacheKey(quint16 hash, int width) {
    return QString("CoverArtCache_%1_%2")
            .arg(QString::number(hash), QString::number(width));
}

// The transformation mode when scaling images
const Qt::TransformationMode kTransformationMode = Qt::SmoothTransformation;

// Resizes the image (preserving aspect ratio) to width.
inline QImage resizeImageWidth(const QImage& image, int width) {
    return image.scaledToWidth(width, kTransformationMode);
}

} // anonymous namespace

CoverArtCache::CoverArtCache() {
    QPixmapCache::setCacheLimit(kPixmapCacheLimit);
}

//static
void CoverArtCache::requestCover(
        const QObject* pRequestor,
        const CoverInfo& coverInfo,
        const TrackPointer& pTrack) {
    CoverArtCache* pCache = CoverArtCache::instance();
    VERIFY_OR_DEBUG_ASSERT(pCache) {
        return;
    }
    pCache->tryLoadCover(
            pRequestor,
            pTrack,
            coverInfo,
            0, // original size
            Loading::Default);
}

//static
void CoverArtCache::requestTrackCover(
        const QObject* pRequestor,
        const TrackPointer& pTrack) {
    VERIFY_OR_DEBUG_ASSERT(pTrack) {
        return;
    }
    requestCover(
            pRequestor,
            pTrack->getCoverInfoWithLocation(),
            pTrack);
}

QPixmap CoverArtCache::tryLoadCover(
        const QObject* pRequestor,
        const TrackPointer& pTrack,
        const CoverInfo& coverInfo,
        int desiredWidth,
        Loading loading) {
    if (kLogger.traceEnabled()) {
        kLogger.trace()
                << "requestCover"
                << pRequestor
                << coverInfo
                << desiredWidth
                << loading;
    }
    DEBUG_ASSERT(!pTrack ||
                pTrack->getLocation() == coverInfo.trackLocation);

    if (coverInfo.type == CoverInfo::NONE) {
        if (loading == Loading::Default) {
            emit coverFound(pRequestor, coverInfo, QPixmap(), coverInfo.hash, false);
        }
        return QPixmap();
    }

    // keep a list of trackIds for which a future is currently running
    // to avoid loading the same picture again while we are loading it
    QPair<const QObject*, quint16> requestId = qMakePair(pRequestor, coverInfo.hash);
    if (m_runningRequests.contains(requestId)) {
        return QPixmap();
    }

    // If this request comes from CoverDelegate (table view), it'll want to get
    // a cropped cover which is ready to be drawn in the table view (cover art
    // column). It's very important to keep the cropped covers in cache because
    // it avoids having to rescale+crop it ALWAYS (which brings a lot of
    // performance issues).
    QString cacheKey = pixmapCacheKey(coverInfo.hash, desiredWidth);

    QPixmap pixmap;
    if (QPixmapCache::find(cacheKey, &pixmap)) {
        if (kLogger.traceEnabled()) {
            kLogger.trace()
                    << "requestCover cache hit"
                    << coverInfo
                    << loading;
        }
        if (loading == Loading::Default) {
            emit coverFound(pRequestor, coverInfo, pixmap, coverInfo.hash, false);
        }
        return pixmap;
    }

    if (loading == Loading::CachedOnly) {
        if (kLogger.traceEnabled()) {
            kLogger.trace() << "requestCover cache miss";
        }
        return QPixmap();
    }

    if (kLogger.traceEnabled()) {
        kLogger.trace()
                << "requestCover starting future for"
                << coverInfo;
    }
    m_runningRequests.insert(requestId);
    // The watcher will be deleted in coverLoaded()
    QFutureWatcher<FutureResult>* watcher = new QFutureWatcher<FutureResult>(this);
    QFuture<FutureResult> future = QtConcurrent::run(
            &CoverArtCache::loadCover,
            pRequestor,
            pTrack,
            coverInfo,
            desiredWidth,
            loading == Loading::Default);
    connect(watcher,
            &QFutureWatcher<FutureResult>::finished,
            this,
            &CoverArtCache::coverLoaded);
    watcher->setFuture(future);
    return QPixmap();
}

//static
CoverArtCache::FutureResult CoverArtCache::loadCover(
        const QObject* pRequestor,
        TrackPointer pTrack,
        CoverInfo coverInfo,
        int desiredWidth,
        bool signalWhenDone) {
    if (kLogger.traceEnabled()) {
        kLogger.trace()
                << "loadCover"
                << coverInfo
                << desiredWidth
                << signalWhenDone;
    }
    DEBUG_ASSERT(!pTrack ||
            pTrack->getLocation() == coverInfo.trackLocation);

    FutureResult res;
    res.pRequestor = pRequestor;
    res.requestedHash = coverInfo.hash;
    res.signalWhenDone = signalWhenDone;
    DEBUG_ASSERT(!res.coverInfoUpdated);

    QImage image = coverInfo.loadImage(
            pTrack ? pTrack->getSecurityToken() : SecurityTokenPointer());

    // Refresh hash before resizing the original image!
    res.coverInfoUpdated = coverInfo.refreshImageHash(image);
    if (pTrack && res.coverInfoUpdated) {
        kLogger.info()
                << "Updating cover info of track"
                << coverInfo.trackLocation;
        pTrack->setCoverInfo(coverInfo);
    }

    // Resize image to requested size
    if (!image.isNull() && desiredWidth > 0) {
        // Adjust the cover size according to the request
        // or downsize the image for efficiency.
        image = resizeImageWidth(image, desiredWidth);
    }

    res.cover = CoverArt(coverInfo, image, desiredWidth);
    return res;
}

// watcher
void CoverArtCache::coverLoaded() {
    FutureResult res;
    {
        QFutureWatcher<FutureResult>* pFutureWatcher =
                static_cast<QFutureWatcher<FutureResult>*>(sender());
        VERIFY_OR_DEBUG_ASSERT(pFutureWatcher) {
            return;
        }
        res = pFutureWatcher->result();
        pFutureWatcher->deleteLater();
    }

    if (kLogger.traceEnabled()) {
        kLogger.trace() << "coverLoaded" << res.cover;
    }

    // Don't cache full size covers (resizedToWidth = 0)
    // Large cover art wastes space in our cache and will likely
    // uncache a lot of the small covers we need in the library
    // table.
    // Full size covers are used in the Skin Widgets, which are
    // loaded with an artificial delay anyway and an additional
    // re-load delay can be accepted.

    // Create pixmap, GUI thread only
    QPixmap pixmap = QPixmap::fromImage(res.cover.image);
    if (!pixmap.isNull() && res.cover.resizedToWidth != 0) {
        // we have to be sure that res.cover.hash is unique
        // because insert replaces the images with the same key
        QString cacheKey = pixmapCacheKey(
                res.cover.hash, res.cover.resizedToWidth);
        QPixmapCache::insert(cacheKey, pixmap);
    }

    m_runningRequests.remove(qMakePair(res.pRequestor, res.requestedHash));

    if (res.signalWhenDone) {
        emit coverFound(res.pRequestor, res.cover, pixmap, res.requestedHash, res.coverInfoUpdated);
    }
}
