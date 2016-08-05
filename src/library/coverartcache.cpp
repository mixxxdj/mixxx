#include <QFutureWatcher>
#include <QPixmapCache>
#include <QStringBuilder>
#include <QtConcurrentRun>
#include <QtDebug>

#include "library/coverartcache.h"
#include "library/coverartutils.h"


namespace {
    QString pixmapCacheKey(quint16 hash, int width) {
        return QString("CoverArtCache_%1_%2")
                .arg(QString::number(hash)).arg(width);
    }

    // The transformation mode when scaling images
    const Qt::TransformationMode kTransformationMode = Qt::SmoothTransformation;

    // Resizes the image (preserving aspect ratio) if the size of width or height
    // exceeds maxEdgeSize.
    inline QImage limitImageSize(const QImage& image, int maxEdgeSize) {
        if ((image.width() > maxEdgeSize) || (image.height() > maxEdgeSize)) {
            return image.scaled(
                    maxEdgeSize, maxEdgeSize,
                    Qt::KeepAspectRatio,
                    kTransformationMode);
        } else {
            return image;
        }
    }

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
        qDebug() << "CoverArtCache::requestCover"
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
            qDebug() << "CoverArtCache::requestCover cache miss";
        }
        return QPixmap();
    }

    m_runningRequests.insert(requestId);
    QFutureWatcher<FutureResult>* watcher = new QFutureWatcher<FutureResult>(this);
    QFuture<FutureResult> future = QtConcurrent::run(
            this, &CoverArtCache::loadCover, requestInfo, pRequestor,
            desiredWidth, signalWhenDone);
    connect(watcher, SIGNAL(finished()), this, SLOT(coverLoaded()));
    watcher->setFuture(future);
    return QPixmap();
}

//static
void CoverArtCache::requestCover(const Track* pTrack,
                         const QObject* pRequestor) {
    CoverArtCache* pCache = CoverArtCache::instance();
    if (pCache == nullptr || pTrack == nullptr) return;

    CoverInfo info = pTrack->getCoverInfo();
    // trackLocation can be still empty here
    // TODO(DSC) it is an design issue that we have
    // redundant trackLocation info here
    info.trackLocation = pTrack->getLocation();
    pCache->requestCover(info, pRequestor, 0, false, true);
}

CoverArtCache::FutureResult CoverArtCache::loadCover(
        const CoverInfo& info,
        const QObject* pRequestor,
        const int desiredWidth,
        const bool signalWhenDone) {
    if (sDebug) {
        qDebug() << "CoverArtCache::loadCover"
                 << info << desiredWidth << signalWhenDone;
    }

    FutureResult res;
    res.pRequestor = pRequestor;
    res.cover.info = info;
    res.desiredWidth = desiredWidth;
    res.signalWhenDone = signalWhenDone;
    res.cover.image = CoverArtUtils::loadCover(res.cover.info);

    if (res.cover.image.isNull()) {
        return res;
    }

    // TODO(XXX) Should we re-hash here? If the cover file (or track metadata)
    // has changed then info.hash may be incorrect. The fix
    // will also require noticing a hash mis-match at higher levels and
    // recording the hash change in the database.

    // Adjust the cover size according to the request or downsize the image for
    // efficiency.
    if (res.desiredWidth > 0) {
        res.cover.image = resizeImageWidth(res.cover.image, res.desiredWidth);
    }

    return res;
}

// watcher
void CoverArtCache::coverLoaded() {
    QFutureWatcher<FutureResult>* watcher;
    watcher = reinterpret_cast<QFutureWatcher<FutureResult>*>(sender());
    FutureResult res = watcher->result();

    if (sDebug) {
        qDebug() << "CoverArtCache::coverLoaded" << res.cover;
    }

    QPixmap pixmap = cacheCover(res.cover, res.desiredWidth);

    m_runningRequests.remove(qMakePair(res.pRequestor, res.cover.info.hash));

    if (res.signalWhenDone) {
        emit(coverFound(res.pRequestor, res.cover.info, pixmap, false));
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
        CoverArt cover = CoverArtUtils::guessCoverArt(pTrack);
        pTrack->setCoverInfo(cover.info);
    }
}

void CoverArtCache::guessCovers(QList<TrackPointer> tracks) {
    qDebug() << "CoverArtCache::guessCovers guessing covers for"
             << tracks.size() << "tracks";
    foreach (TrackPointer pTrack, tracks) {
        guessCover(pTrack);
    }
}

QPixmap CoverArtCache::cacheCover(CoverArt cover, int width) {
    QPixmap pixmap;
    if (!cover.image.isNull()) {
        QString cacheKey = pixmapCacheKey(cover.info.hash, width);
        if (!QPixmapCache::find(cacheKey, &pixmap)) {
            pixmap.convertFromImage(cover.image);
            // Don't cache full size covers (Width = 0)
            // Large cover art wastes space in our cache and will likely
            // uncache a lot of the small covers we need in the library
            // table.
            // Full size covers are used in the Skin Widgets, which are
            // loaded with an artificial delay anyway and an additional
            // re-load delay can be accepted.
            if (width > 0) {
                QPixmapCache::insert(cacheKey, pixmap);
            }
        }
    }
    return pixmap;
}
