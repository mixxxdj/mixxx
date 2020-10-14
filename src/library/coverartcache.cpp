#include "library/coverartcache.h"

#include <QFutureWatcher>
#include <QPixmapCache>
#include <QtConcurrentRun>
#include <QtDebug>

#include "library/coverartutils.h"
#include "track/track.h"
#include "util/compatibility.h"
#include "util/logger.h"
#include "util/thread_affinity.h"

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

    auto loadedImage = coverInfo.loadImage(
            pTrack ? pTrack->getSecurityToken() : SecurityTokenPointer());
    if (!loadedImage.image.isNull()) {
        // Refresh hash before resizing the original image!
        res.coverInfoUpdated = coverInfo.refreshImageHash(loadedImage.image);
        if (pTrack && res.coverInfoUpdated) {
            kLogger.info()
                    << "Updating cover info of track"
                    << coverInfo.trackLocation;
            pTrack->setCoverInfo(coverInfo);
        }

        // Resize image to requested size
        if (desiredWidth > 0) {
            // Adjust the cover size according to the request
            // or downsize the image for efficiency.
            loadedImage.image = resizeImageWidth(loadedImage.image, desiredWidth);
        }
    }

    res.coverArt = CoverArt(
            std::move(coverInfo),
            std::move(loadedImage),
            desiredWidth);
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
        kLogger.trace() << "coverLoaded" << res.coverArt;
    }

    QPixmap pixmap;
    if (res.coverArt.loadedImage.result != CoverInfo::LoadedImage::Result::NoImage) {
        if (res.coverArt.loadedImage.result == CoverInfo::LoadedImage::Result::Ok) {
            DEBUG_ASSERT(!res.coverArt.loadedImage.filePath.isEmpty());
        } else {
            DEBUG_ASSERT(res.coverArt.loadedImage.image.isNull());
            kLogger.warning()
                    << "Failed to load cover art image"
                    << res.coverArt.loadedImage
                    << "for track"
                    << res.coverArt.trackLocation;
            // Substitute missing cover art with a placeholder image to avoid high CPU load
            // See also: https://bugs.launchpad.net/mixxx/+bug/1879160
            const int imageSize = math_max(1, res.coverArt.resizedToWidth);
            QImage placeholderImage(imageSize, imageSize, QImage::Format_RGB32);
            // TODO(uklotzde): Use optional cover art background color (if available)
            // instead of Qt::darkGray
            placeholderImage.fill(
                    mixxx::RgbColor::toQColor(std::nullopt /*res.coverArt.color*/, Qt::darkGray));
            res.coverArt.loadedImage.image = placeholderImage;
        }
        // Create pixmap, GUI thread only!
        DEBUG_ASSERT_MAIN_THREAD_AFFINITY();
        DEBUG_ASSERT(!res.coverArt.loadedImage.image.isNull());
        pixmap = QPixmap::fromImage(res.coverArt.loadedImage.image);
        // Don't cache full size covers (resizedToWidth = 0)
        // Large cover art wastes space in our cache and will likely
        // uncache a lot of the small covers we need in the library
        // table.
        // Full size covers are used in the Skin Widgets, which are
        // loaded with an artificial delay anyway and an additional
        // re-load delay can be accepted.
        if (res.coverArt.resizedToWidth > 0) {
            DEBUG_ASSERT(!pixmap.isNull());
            // It is very unlikely that res.coverArt.hash generates the
            // same hash for different images. Otherwise the wrong image would
            // be displayed when loaded from the cache.
            QString cacheKey = pixmapCacheKey(
                    res.coverArt.hash, res.coverArt.resizedToWidth);
            QPixmapCache::insert(cacheKey, pixmap);
        }
    }

    m_runningRequests.remove(qMakePair(res.pRequestor, res.requestedHash));

    if (res.signalWhenDone) {
        emit coverFound(
                res.pRequestor,
                std::move(res.coverArt),
                pixmap,
                res.requestedHash,
                res.coverInfoUpdated);
    }
}
