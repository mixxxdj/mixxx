#include <QFutureWatcher>
#include <QPixmapCache>
#include <QStringBuilder>
#include <QtConcurrentRun>
#include <QtDebug>

#include "library/coverartcache.h"
#include "library/coverartutils.h"
#include "util/logger.h"


namespace {

mixxx::Logger kLogger("CoverArtCache");

QString pixmapCacheKey(quint16 hash, int width) {
    return QString("CoverArtCache_%1_%2")
            .arg(QString::number(hash)).arg(width);
}

// The transformation mode when scaling images
const Qt::TransformationMode kTransformationMode = Qt::SmoothTransformation;

// Resizes the image (preserving aspect ratio) to width.
inline QImage resizeImageWidth(const QImage& image, int width) {
    return image.scaledToWidth(width, kTransformationMode);
}

} // anonymous namespace

const bool sDebug = false;

CoverArtCache::CoverArtCache() {
    // The initial QPixmapCache limit is 10MB.
    // But it is not used just by the coverArt stuff,
    // it is also used by Qt to handle other things behind the scenes.
    // Consequently coverArt cache will always have less than those
    // 10MB available to store the pixmaps.
    // So, we must increase this size a bit more,
    // in order to allow CoverCache handle more covers (performance gain).
    QPixmapCache::setCacheLimit(20480);
}

CoverArtCache::~CoverArtCache() {
    qDebug() << "~CoverArtCache()";
}

QPixmap CoverArtCache::requestCover(const CoverInfo& requestInfo,
                                    const QObject* pRequestor,
                                    const int desiredWidth,
                                    const bool onlyCached,
                                    const bool signalWhenDone) {
    if (sDebug) {
        kLogger.debug() << "requestCover"
                 << requestInfo << pRequestor <<
                desiredWidth << onlyCached << signalWhenDone;
    }

    if (requestInfo.type == CoverInfo::NONE) {
        if (signalWhenDone) {
            emit(coverFound(pRequestor, requestInfo,
                            QPixmap(), true));
        }
        return QPixmap();
    }

    // keep a list of trackIds for which a future is currently running
    // to avoid loading the same picture again while we are loading it
    QPair<const QObject*, quint16> requestId = qMakePair(pRequestor, requestInfo.hash);
    if (m_runningRequests.contains(requestId)) {
        return QPixmap();
    }

    // If this request comes from CoverDelegate (table view), it'll want to get
    // a cropped cover which is ready to be drawn in the table view (cover art
    // column). It's very important to keep the cropped covers in cache because
    // it avoids having to rescale+crop it ALWAYS (which brings a lot of
    // performance issues).
    QString cacheKey = pixmapCacheKey(requestInfo.hash, desiredWidth);

    QPixmap pixmap;
    if (QPixmapCache::find(cacheKey, &pixmap)) {
        if (signalWhenDone) {
            emit(coverFound(pRequestor, requestInfo, pixmap, true));
        }
        return pixmap;
    }

    if (onlyCached) {
        if (sDebug) {
            kLogger.debug() << "requestCover cache miss";
        }
        return QPixmap();
    }

    m_runningRequests.insert(requestId);
    // The watcher will be deleted in coverLoaded()
    QFutureWatcher<FutureResult>* watcher = new QFutureWatcher<FutureResult>(this);
    QFuture<FutureResult> future = QtConcurrent::run(
            this, &CoverArtCache::loadCover, requestInfo, pRequestor,
            desiredWidth, signalWhenDone);
    connect(watcher, SIGNAL(finished()), this, SLOT(coverLoaded()));
    watcher->setFuture(future);
    return QPixmap();
}

//static
void CoverArtCache::requestCover(const Track& track,
                         const QObject* pRequestor) {
    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache == nullptr) return;

    CoverInfo info = track.getCoverInfoWithLocation();
    pCache->requestCover(info, pRequestor, 0, false, true);
}

CoverArtCache::FutureResult CoverArtCache::loadCover(
        const CoverInfo& info,
        const QObject* pRequestor,
        const int desiredWidth,
        const bool signalWhenDone) {
    if (sDebug) {
        kLogger.debug() << "loadCover"
                 << info << desiredWidth << signalWhenDone;
    }

    QImage image = CoverArtUtils::loadCover(info);

    // TODO(XXX) Should we re-hash here? If the cover file (or track metadata)
    // has changed then info.hash may be incorrect. The fix
    // will also require noticing a hash mis-match at higher levels and
    // recording the hash change in the database.

    // Adjust the cover size according to the request or downsize the image for
    // efficiency.
    if (!image.isNull() && desiredWidth > 0) {
        image = resizeImageWidth(image, desiredWidth);
    }

    FutureResult res;
    res.pRequestor = pRequestor;
    res.cover = CoverArt(info, image, desiredWidth);
    res.signalWhenDone = signalWhenDone;

    return res;
}

// watcher
void CoverArtCache::coverLoaded() {
    FutureResult res;
    {
        QFutureWatcher<FutureResult>* watcher =
                static_cast<QFutureWatcher<FutureResult>*>(sender());
        res = watcher->result();
        watcher->deleteLater();
    }

    if (sDebug) {
        kLogger.debug() << "coverLoaded" << res.cover;
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

    m_runningRequests.remove(qMakePair(res.pRequestor, res.cover.hash));

    if (res.signalWhenDone) {
        emit(coverFound(res.pRequestor, res.cover, pixmap, false));
    }
}

void CoverArtCache::requestGuessCovers(QList<TrackPointer> tracks) {
    QtConcurrent::run(this, &CoverArtCache::guessCovers, tracks);
}

void CoverArtCache::requestGuessCover(TrackPointer pTrack) {
    QtConcurrent::run(this, &CoverArtCache::guessCover, pTrack);
}

void CoverArtCache::guessCover(TrackPointer pTrack) {
    if (pTrack) {
        if (kLogger.debugEnabled()) {
            kLogger.debug()
                    << "Guessing cover art for"
                    << pTrack->getFileInfo();
        }
        CoverInfo cover = CoverArtUtils::guessCoverInfo(*pTrack);
        pTrack->setCoverInfo(cover);
    }
}

void CoverArtCache::guessCovers(QList<TrackPointer> tracks) {
    if (kLogger.debugEnabled()) {
        kLogger.debug()
                << "Guessing cover art for"
                << tracks.size()
                << "tracks";
    }
    foreach (TrackPointer pTrack, tracks) {
        guessCover(pTrack);
    }
}
