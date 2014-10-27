#include <QFutureWatcher>
#include <QPixmapCache>
#include <QStringBuilder>
#include <QtConcurrentRun>
#include <QtDebug>

#include "library/coverartcache.h"
#include "library/coverartutils.h"
#include "soundsourceproxy.h"

// Large cover art wastes space in our cache when we typicaly won't show them at
// their full size. This is the max side length we resize images to.
const int kMaxCoverSize = 300;

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
                                    const QSize& croppedSize,
                                    const bool onlyCached,
                                    const bool issueRepaint) {
    if (sDebug) {
        qDebug() << "CoverArtCache::requestCover"
                 << requestInfo.coverLocation
                 << requestInfo.hash
                 << requestInfo.trackId
                 << requestInfo.trackLocation;
    }

    // TODO(XXX) handle requests for non-library tracks.
    if (requestInfo.trackId < 1) {
        return QPixmap();
    }

    // keep a list of trackIds for which a future is currently running
    // to avoid loading the same picture again while we are loading it
    if (m_runningIds.contains(requestInfo.trackId)) {
        return QPixmap();
    }

    // Begin to build the request.
    CoverAndAlbumInfo coverAndAlbumInfo;
    coverAndAlbumInfo.info = requestInfo;

    // If this request comes from CoverDelegate (table view),
    // it'll want to get a cropped cover which is ready to be drawn
    // in the table view (cover art column).
    // It's very important to keep the cropped covers in cache because it avoids
    // having to rescale+crop it ALWAYS (which brings a lot of performance issues).
    if (!coverAndAlbumInfo.info.hash.isEmpty()) {
        QString cacheKey = CoverArtUtils::pixmapCacheKey(coverAndAlbumInfo.info.hash,
                                                         croppedSize);

        QPixmap pixmap;
        if (QPixmapCache::find(cacheKey, &pixmap)) {
            if (!issueRepaint) {
                emit(pixmapFound(coverAndAlbumInfo.info.trackId, pixmap));
            }
            return pixmap;
        }
    }

    if (onlyCached) {
        if (sDebug) {
            qDebug() << "CoverArtCache::requestCover cache miss";
        }
        return QPixmap();
    }

    m_runningIds.insert(coverAndAlbumInfo.info.trackId);
    QFutureWatcher<FutureResult>* watcher = new QFutureWatcher<FutureResult>(this);
    QFuture<FutureResult> future = QtConcurrent::run(
            this, &CoverArtCache::loadCover, coverAndAlbumInfo, croppedSize,
            issueRepaint);
    connect(watcher, SIGNAL(finished()), this, SLOT(coverLoaded()));
    watcher->setFuture(future);
    return QPixmap();
}

CoverArtCache::FutureResult CoverArtCache::loadCover(
        const CoverAndAlbumInfo& coverAndAlbumInfo,
        const QSize& croppedSize,
        const bool issueRepaint) {
    if (sDebug) {
        qDebug() << "CoverArtCache::loadCover"
                 << coverAndAlbumInfo.info.coverLocation
                 << coverAndAlbumInfo.info.hash
                 << coverAndAlbumInfo.info.trackId
                 << coverAndAlbumInfo.info.trackLocation
                 << coverAndAlbumInfo.album;
    }

    FutureResult res;
    res.cover.info = coverAndAlbumInfo.info;
    res.croppedSize = croppedSize;
    res.issueRepaint = issueRepaint;
    res.cover.image = CoverArtUtils::loadCover(res.cover.info);

    if (res.cover.image.isNull()) {
        return res;
    }

    // TODO(XXX) Should we re-hash here? If the cover file (or track metadata)
    // has changed then coverAndAlbumInfo.info.hash may be incorrect. The fix
    // will also require noticing a hash mis-match at higher levels and
    // recording the hash change in the database.

    // Adjust the cover size according to the request or downsize the image for
    // efficiency.
    if (res.croppedSize.isNull()) {
        res.cover.image = CoverArtUtils::maybeResizeImage(res.cover.image,
                                                          kMaxCoverSize);
    } else {
        res.cover.image = CoverArtUtils::cropImage(res.cover.image,
                                                   res.croppedSize);
    }

    return res;
}

// watcher
void CoverArtCache::coverLoaded() {
    QFutureWatcher<FutureResult>* watcher;
    watcher = reinterpret_cast<QFutureWatcher<FutureResult>*>(sender());
    FutureResult res = watcher->result();

    if (sDebug) {
        qDebug() << "CoverArtCache::coverLoaded"
                 << res.cover.info.coverLocation
                 << res.cover.info.hash
                 << res.cover.info.trackId
                 << res.cover.info.trackLocation
                 << res.cover.image.isNull();
    }

    QString cacheKey = CoverArtUtils::pixmapCacheKey(res.cover.info.hash,
                                                     res.croppedSize);
    QPixmap pixmap;
    if (!QPixmapCache::find(cacheKey, &pixmap) && !res.cover.image.isNull()) {
        pixmap.convertFromImage(res.cover.image);
        QPixmapCache::insert(cacheKey, pixmap);
    }

    if (res.issueRepaint) {
        emit(requestRepaint());
    } else {
        emit(pixmapFound(res.cover.info.trackId, pixmap));
    }

    m_runningIds.remove(res.cover.info.trackId);
}
