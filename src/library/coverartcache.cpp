#include <QFutureWatcher>
#include <QPixmapCache>
#include <QStringBuilder>
#include <QtConcurrentRun>
#include <QtDebug>

#include "library/coverartcache.h"
#include "library/coverartutils.h"
#include "soundsourceproxy.h"

// Large cover art wastes space in our cache when we typicaly won't show them at
// their full size. If no width is specified, this is the maximum width cap.
const int kMaxCoverWidth = 300;

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
                                    int requestReference,
                                    const int desiredWidth,
                                    const bool onlyCached,
                                    const bool signalWhenDone) {
    if (sDebug) {
        qDebug() << "CoverArtCache::requestCover"
                 << requestInfo << pRequestor << requestReference <<
                desiredWidth << onlyCached << signalWhenDone;
    }

    if (requestInfo.type == CoverInfo::NONE) {
        if (signalWhenDone) {
            emit(coverFound(pRequestor, requestReference, requestInfo,
                            QPixmap(), true));
        }
        return QPixmap();
    }

    // keep a list of trackIds for which a future is currently running
    // to avoid loading the same picture again while we are loading it
    QPair<const QObject*, int> requestId = qMakePair(pRequestor, requestReference);
    if (m_runningRequests.contains(requestId)) {
        return QPixmap();
    }

    // If this request comes from CoverDelegate (table view), it'll want to get
    // a cropped cover which is ready to be drawn in the table view (cover art
    // column). It's very important to keep the cropped covers in cache because
    // it avoids having to rescale+crop it ALWAYS (which brings a lot of
    // performance issues).
    QString cacheKey = CoverArtUtils::pixmapCacheKey(requestInfo.hash,
                                                     desiredWidth);

    QPixmap pixmap;
    if (QPixmapCache::find(cacheKey, &pixmap)) {
        if (signalWhenDone) {
            emit(coverFound(pRequestor, requestReference, requestInfo, pixmap, true));
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
            requestReference, desiredWidth, signalWhenDone);
    connect(watcher, SIGNAL(finished()), this, SLOT(coverLoaded()));
    watcher->setFuture(future);
    return QPixmap();
}

CoverArtCache::FutureResult CoverArtCache::loadCover(
        const CoverInfo& info,
        const QObject* pRequestor,
        int requestReference,
        const int desiredWidth,
        const bool signalWhenDone) {
    if (sDebug) {
        qDebug() << "CoverArtCache::loadCover"
                 << info << desiredWidth << signalWhenDone;
    }

    FutureResult res;
    res.pRequestor = pRequestor;
    res.requestReference = requestReference;
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
        res.cover.image = CoverArtUtils::resizeImage(res.cover.image,
                                                     res.desiredWidth);
    } else {
        res.cover.image = CoverArtUtils::maybeResizeImage(res.cover.image,
                                                          kMaxCoverWidth);
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

    QString cacheKey = CoverArtUtils::pixmapCacheKey(res.cover.info.hash,
                                                     res.desiredWidth);
    QPixmap pixmap;
    if (!QPixmapCache::find(cacheKey, &pixmap) && !res.cover.image.isNull()) {
        pixmap.convertFromImage(res.cover.image);
        QPixmapCache::insert(cacheKey, pixmap);
    }
    m_runningRequests.remove(qMakePair(res.pRequestor, res.requestReference));

    if (res.signalWhenDone) {
        emit(coverFound(res.pRequestor, res.requestReference,
                        res.cover.info, pixmap, false));
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
        pTrack->setCoverArt(CoverArtUtils::guessCoverArt(pTrack));
    }
}

void CoverArtCache::guessCovers(QList<TrackPointer> tracks) {
    qDebug() << "CoverArtCache::guessCovers guessing covers for"
             << tracks.size() << "tracks";
    foreach (TrackPointer pTrack, tracks) {
        guessCover(pTrack);
    }
}
