#include "library/coverartcache.h"

#include <QFutureWatcher>
#include <QPixmapCache>
#include <QtConcurrentRun>
#include <QtDebug>

#include "moc_coverartcache.cpp"
#include "track/track.h"
#include "util/logger.h"
#include "util/thread_affinity.h"

namespace {

mixxx::Logger kLogger("CoverArtCache");

QString pixmapCacheKey(mixxx::cache_key_t hash, int width) {
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
}

//static
void CoverArtCache::requestCoverImpl(
        const QObject* pRequester,
        const TrackPointer& pTrack,
        const CoverInfo& coverInfo,
        int desiredWidth) {
    CoverArtCache* pCache = CoverArtCache::instance();
    VERIFY_OR_DEBUG_ASSERT(pCache) {
        return;
    }
    QPixmap pixmap = CoverArtCache::getCachedCover(coverInfo, desiredWidth);
    if (!pixmap.isNull()) {
        emit pCache->coverFound(pRequester, coverInfo, pixmap);
        return;
    }
    pCache->tryLoadCover(
            pRequester,
            pTrack,
            coverInfo,
            desiredWidth);
}

//static
void CoverArtCache::requestTrackCover(
        const QObject* pRequester,
        const TrackPointer& pTrack) {
    VERIFY_OR_DEBUG_ASSERT(pTrack) {
        return;
    }
    requestCoverImpl(
            pRequester,
            pTrack,
            pTrack->getCoverInfoWithLocation());
}

// static
QPixmap CoverArtCache::getCachedCover(
        const CoverInfo& coverInfo,
        int desiredWidth) {
    if (!coverInfo.hasImage()) {
        return QPixmap();
    }
    const mixxx::cache_key_t requestedCacheKey = coverInfo.cacheKey();
    QString cacheKey = pixmapCacheKey(requestedCacheKey, desiredWidth);

    QPixmap pixmap;
    if (!QPixmapCache::find(cacheKey, &pixmap)) {
        if (kLogger.traceEnabled()) {
            kLogger.trace()
                    << "requestCover cache miss"
                    << coverInfo;
        }
        return QPixmap();
    }

    if (kLogger.traceEnabled()) {
        kLogger.trace()
                << "requestCover cache hit"
                << coverInfo;
    }
    return pixmap;
}

// static
void CoverArtCache::requestUncachedCover(
        const QObject* pRequester,
        const CoverInfo& coverInfo,
        int desiredWidth) {
    CoverArtCache* pCache = CoverArtCache::instance();
    VERIFY_OR_DEBUG_ASSERT(pCache) {
        return;
    }
    pCache->tryLoadCover(
            pRequester,
            TrackPointer(),
            coverInfo,
            desiredWidth);
}

// static
void CoverArtCache::requestUncachedCover(
        const QObject* pRequester,
        const TrackPointer& pTrack,
        int desiredWidth) {
    VERIFY_OR_DEBUG_ASSERT(pTrack) {
        return;
    }

    CoverArtCache* pCache = CoverArtCache::instance();
    VERIFY_OR_DEBUG_ASSERT(pCache) {
        return;
    }
    pCache->tryLoadCover(
            pRequester,
            pTrack,
            pTrack->getCoverInfoWithLocation(),
            desiredWidth);
}

void CoverArtCache::tryLoadCover(
        const QObject* pRequester,
        const TrackPointer& pTrack,
        const CoverInfo& coverInfo,
        int desiredWidth) {
    if (kLogger.traceEnabled()) {
        kLogger.trace()
                << "requestCover"
                << pRequester
                << coverInfo
                << desiredWidth;
    }
    DEBUG_ASSERT(!pTrack ||
                pTrack->getLocation() == coverInfo.trackLocation);

    if (!coverInfo.hasImage()) {
        emit coverFound(pRequester, coverInfo, QPixmap());
        return;
    }

    const mixxx::cache_key_t requestedCacheKey = coverInfo.cacheKey();
    // keep a list of cache keys for which a future is currently running
    // to avoid loading the same picture again while we are loading it.
    // This fixes also https://github.com/mixxxdj/mixxx/issues/11131 on
    // Windows where simultaneous open the same file from two threads fails.
    bool requestPending = m_runningRequests.contains(requestedCacheKey);
    m_runningRequests.insert(requestedCacheKey, {pRequester, desiredWidth});
    if (requestPending) {
        return;
    }

    if (kLogger.traceEnabled()) {
        kLogger.trace()
                << "requestCover starting future for"
                << coverInfo;
    }

    // The watcher will be deleted in coverLoaded()
    QFutureWatcher<FutureResult>* watcher = new QFutureWatcher<FutureResult>(this);
    QFuture<FutureResult> future = QtConcurrent::run(
            &CoverArtCache::loadCover,
            pTrack,
            coverInfo,
            desiredWidth);
    connect(watcher,
            &QFutureWatcher<FutureResult>::finished,
            this,
            &CoverArtCache::coverLoaded);
    watcher->setFuture(future);
    return;
}

//static
CoverArtCache::FutureResult CoverArtCache::loadCover(
        TrackPointer pTrack,
        CoverInfo coverInfo,
        int desiredWidth) {
    if (kLogger.traceEnabled()) {
        kLogger.trace()
                << "loadCover"
                << coverInfo
                << desiredWidth;
    }
    DEBUG_ASSERT(!pTrack ||
            pTrack->getLocation() == coverInfo.trackLocation);

    auto res = FutureResult(
            coverInfo.cacheKey());

    CoverInfo::LoadedImage loadedImage = coverInfo.loadImage(pTrack);
    if (!loadedImage.image.isNull()) {
        if (coverInfo.imageDigest().isEmpty()) {
            // This happens if we have loaded the cover art via the legacy hash
            // and during tests.
            // Refresh hash before resizing the original image!
            if (pTrack) {
                CoverInfo updatedCoverInfo = coverInfo;
                updatedCoverInfo.setImageDigest(loadedImage.image);
                kLogger.info()
                        << "Updating cover info of track"
                        << coverInfo.trackLocation;
                pTrack->setCoverInfo(updatedCoverInfo);
            }
        }

        // Resize image to requested size
        if (desiredWidth > 0) {
            // Adjust the cover size according to the request
            // or downsize the image for efficiency.
            loadedImage.image = resizeImageWidth(loadedImage.image, desiredWidth);
        }
    } else {
        kLogger.warning() << "loaded image is NULL";
        if (pTrack && coverInfo.type == CoverInfo::Type::METADATA) {
            // The image was supposed to be in the track's metadata but is not.
            // Removed form file? Database corruption?
            // Either way this can cause repeated lookups by CoverArtDelegate
            // which are affecting the GUI and cause massive framerate drop,
            // see https://github.com/mixxxdj/mixxx/issues/15199
            // In order to avoid this we reset the track's cover info.
            // On next cover request we'll try to guess the cover again, so we
            // either find a cover in the track directory or mark CoverInfo empty.
            kLogger.warning() << "image was expected to be in metadata, but it's not. "
                                 "Reset track's cover info";
            pTrack->setCoverInfo(CoverInfoRelative());
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

    QString cacheKey = pixmapCacheKey(
            res.coverArt.cacheKey(), res.coverArt.resizedToWidth);
    QPixmap pixmap;
    if (res.coverArt.loadedImage.result != CoverInfo::LoadedImage::Result::NoImage) {
        if (res.coverArt.loadedImage.result == CoverInfo::LoadedImage::Result::Ok) {
            DEBUG_ASSERT(!res.coverArt.loadedImage.location.isEmpty());
        } else {
            DEBUG_ASSERT(res.coverArt.loadedImage.image.isNull());
            kLogger.warning()
                    << "Failed to load cover art image"
                    << res.coverArt.loadedImage
                    << "for track"
                    << res.coverArt.trackLocation;
            // Substitute missing cover art with a placeholder image to avoid high CPU load
            // See also: https://github.com/mixxxdj/mixxx/issues/9974
            const int imageSize = math_max(1, res.coverArt.resizedToWidth);
            QImage placeholderImage(imageSize, imageSize, QImage::Format_RGB32);
            placeholderImage.fill(
                    mixxx::RgbColor::toQColor(res.coverArt.color, Qt::darkGray));
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
            QPixmapCache::insert(cacheKey, pixmap);
        }
    }

    auto runningRequests = m_runningRequests;
    // First remove all requests for this cover that way we can
    // re-add cover with different sizes via tryLoadCover() as usual
    m_runningRequests.remove(res.coverArt.cacheKey());

    auto i = runningRequests.find(res.coverArt.cacheKey());
    while (i != runningRequests.end() && i.key() == res.coverArt.cacheKey()) {
        if (i.value().desiredWidth == res.coverArt.resizedToWidth) {
            emit coverFound(
                    i.value().pRequester,
                    res.coverArt,
                    pixmap);
        } else {
            tryLoadCover(
                    i.value().pRequester,
                    nullptr,
                    res.coverArt,
                    i.value().desiredWidth);
        }
        ++i;
    }
}
