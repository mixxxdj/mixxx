#include <QFutureWatcher>
#include <QPixmapCache>
#include <QStringBuilder>
#include <QtConcurrentRun>
#include <QtDebug>

#include "library/coverartcache.h"
#include "library/coverartutils.h"
#include "library/dao/coverartdao.h"
#include "library/dao/trackdao.h"
#include "soundsourceproxy.h"

// Large cover art wastes space in our cache when we typicaly won't show them at
// their full size. This is the max side length we resize images to.
const int kMaxCoverSize = 300;

const bool sDebug = false;

CoverArtCache::CoverArtCache()
        : m_pCoverArtDAO(NULL),
          m_pTrackDAO(NULL) {
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

    // The queue of updates might have some covers/tracks
    // waiting for db insert.
    // So, we force the update in order to save everything
    // before the destruction...
    updateDB(true);
}

void CoverArtCache::setCoverArtDAO(CoverArtDAO* coverdao) {
    m_pCoverArtDAO = coverdao;
}

void CoverArtCache::setTrackDAO(TrackDAO* trackdao) {
    m_pTrackDAO = trackdao;
}

QString CoverArtCache::trackInDBHash(int trackId) {
    return m_queueOfUpdates.value(trackId).first;
}

bool CoverArtCache::changeCoverArt(int trackId,
                                   const QString& newCoverLocation) {
    if (trackId < 1 || m_pCoverArtDAO == NULL || m_pTrackDAO == NULL) {
        return false;
    }

    m_queueOfUpdates.remove(trackId);

    if (newCoverLocation.isEmpty()) {
        m_pTrackDAO->updateCoverArt(trackId, -1);
        return true;
    }

    QImage img;
    if (newCoverLocation == "ID3TAG") {
        QString trackLocation = m_pTrackDAO->getTrackLocation(trackId);
        img = CoverArtUtils::extractEmbeddedCover(trackLocation);
    } else {
        img = QImage(newCoverLocation);
    }

    if (img.isNull()) {
        return false;
    }

    // Calculate the hash of the full-size image.
    QString hash = CoverArtUtils::calculateHash(img);

    // Maybe resize it for efficiency.
    img = CoverArtUtils::maybeResizeImage(img, kMaxCoverSize);

    // Update DB
    int coverId = m_pCoverArtDAO->saveCoverArt(newCoverLocation, hash);
    m_pTrackDAO->updateCoverArt(trackId, coverId);

    // TODO(XXX): invalidate existing cached cropped pixmaps of this hash
    QString cacheKey = CoverArtUtils::pixmapCacheKey(hash, QSize());
    QPixmap pixmap;
    if (!QPixmapCache::find(cacheKey, &pixmap)) {
        pixmap.convertFromImage(img);
        QPixmapCache::insert(cacheKey, pixmap);
    }
    emit(pixmapFound(trackId, pixmap));
    emit(requestRepaint());
    return true;
}

QPixmap CoverArtCache::requestPixmap(const CoverInfo& requestInfo,
                                     const QSize& croppedSize,
                                     const bool onlyCached,
                                     const bool issueRepaint) {
    if (sDebug) {
        qDebug() << "CoverArtCache::requestPixmap"
                 << requestInfo.coverLocation
                 << requestInfo.hash
                 << requestInfo.trackId
                 << requestInfo.trackLocation;
    }

    // TODO(XXX) handle requests for non-library tracks.
    if (requestInfo.trackId < 1 || m_pCoverArtDAO == NULL) {
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

    // check if we have already found a cover for this track
    // and if it is just waiting to be inserted/updated in the DB.
    // In this case, we update the coverLocation and the hash.
    QPair<QString, QString> update = m_queueOfUpdates.value(
        coverAndAlbumInfo.info.trackId);
    if (!update.first.isEmpty()) {
        coverAndAlbumInfo.info.coverLocation = update.first;
        coverAndAlbumInfo.info.hash = update.second;
    }

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
        return QPixmap();
    }

    QFuture<FutureResult> future;
    QFutureWatcher<FutureResult>* watcher = new QFutureWatcher<FutureResult>(this);
    if (coverAndAlbumInfo.info.hash.isEmpty() ||
           (coverAndAlbumInfo.info.coverLocation != "ID3TAG" &&
            !QFile::exists(coverAndAlbumInfo.info.coverLocation))) {
        coverAndAlbumInfo = m_pCoverArtDAO->getCoverAndAlbumInfo(coverAndAlbumInfo.info.trackId);
        future = QtConcurrent::run(this, &CoverArtCache::searchImage,
                                   coverAndAlbumInfo, croppedSize, issueRepaint);
        connect(watcher, SIGNAL(finished()), this, SLOT(imageFound()));
    } else {
        future = QtConcurrent::run(this, &CoverArtCache::loadImage,
                                   coverAndAlbumInfo, croppedSize, issueRepaint);
        connect(watcher, SIGNAL(finished()), this, SLOT(imageLoaded()));
    }
    m_runningIds.insert(coverAndAlbumInfo.info.trackId);
    watcher->setFuture(future);

    return QPixmap();
}

// Load cover from path stored in DB.
// It is executed in a separate thread via QtConcurrent::run
CoverArtCache::FutureResult CoverArtCache::loadImage(
        const CoverAndAlbumInfo& coverAndAlbumInfo,
        const QSize& croppedSize,
        const bool issueRepaint) {
    if (sDebug) {
        qDebug() << "CoverArtCache::loadImage"
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

    if (res.cover.info.coverLocation == "ID3TAG") {
        res.cover.image = CoverArtUtils::extractEmbeddedCover(
            res.cover.info.trackLocation);
    } else {
        res.cover.image = QImage(res.cover.info.coverLocation);
    }

    // TODO(XXX) Should we re-hash here? If the cover file (or track metadata)
    // has changed then coverAndAlbumInfo.info.hash may be incorrect. The fix
    // will also require noticing a hash mis-match at higher levels and
    // recording the hash change in the database.

    // Adjust the cover size according to the request or downsize the image for
    // efficiency.
    if (res.croppedSize.isNull()) {
        res.cover.image = CoverArtUtils::maybeResizeImage(res.cover.image, kMaxCoverSize);
    } else {
        res.cover.image = CoverArtUtils::cropImage(res.cover.image, res.croppedSize);
    }

    return res;
}

// watcher
void CoverArtCache::imageLoaded() {
    QFutureWatcher<FutureResult>* watcher;
    watcher = reinterpret_cast<QFutureWatcher<FutureResult>*>(sender());
    FutureResult res = watcher->result();

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

// Searching and loading QImages is a very slow process
// that could block the main thread. Therefore, this method
// is executed in a separate thread via QtConcurrent::run
CoverArtCache::FutureResult CoverArtCache::searchImage(
        const CoverAndAlbumInfo& coverAndAlbumInfo,
        const QSize& croppedSize,
        const bool issueRepaint) {
    if (sDebug) {
        qDebug() << "CoverArtCache::searchImage"
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

    // Looking for embedded cover art.
    res.cover.image = CoverArtUtils::extractEmbeddedCover(coverAndAlbumInfo.info.trackLocation);
    if (!res.cover.image.isNull()) {
        res.cover.info.coverLocation = "ID3TAG";
    }

    // Looking for cover stored in track diretory.
    if (res.cover.image.isNull()) {
        QFileInfo track(coverAndAlbumInfo.info.trackLocation);
        QString trackDirectory = track.path();
        QString trackBaseName = track.baseName();
        // TODO(XXX) instead of returning a single location, provide a list of
        // candidates. The returned image may be invalid.
        res.cover.info.coverLocation = CoverArtUtils::searchInTrackDirectory(
            trackDirectory, trackBaseName, coverAndAlbumInfo.album);
        res.cover.image = QImage(res.cover.info.coverLocation);
    }

    if (!res.cover.image.isNull()) {
        // Calculate the hash of the full-size image.
        res.cover.info.hash = CoverArtUtils::calculateHash(res.cover.image);

        // Adjust the cover size according to the request or downsize the image
        // for efficiency.
        if (res.croppedSize.isNull()) {
            res.cover.image = CoverArtUtils::maybeResizeImage(res.cover.image, kMaxCoverSize);
        } else {
            res.cover.image = CoverArtUtils::cropImage(res.cover.image, res.croppedSize);
        }
    } else {
        // Reset the hash to empty if we didn't find anything.
        res.cover.info.hash = QString();
    }

    return res;
}

// watcher
void CoverArtCache::imageFound() {
    QFutureWatcher<FutureResult>* watcher;
    watcher = reinterpret_cast<QFutureWatcher<FutureResult>*>(sender());
    FutureResult res = watcher->result();


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

    // update DB
    if (!m_queueOfUpdates.contains(res.cover.info.trackId)) {
        m_queueOfUpdates.insert(res.cover.info.trackId,
                                qMakePair(res.cover.info.coverLocation,
                                          res.cover.info.hash));
        updateDB();
    }

    m_runningIds.remove(res.cover.info.trackId);
}

// sqlite can't do a huge number of updates in a very short time,
// so it is important to collect all new covers and write them at once.
void CoverArtCache::updateDB(bool forceUpdate) {
    if (!m_pCoverArtDAO || !m_pTrackDAO ||
            (!forceUpdate && m_queueOfUpdates.size() < 500)) {
        return;
    }

    qDebug() << "CoverCache : Updating" << m_queueOfUpdates.size() << "tracks!";

    QSet<QPair<int, int> > covers;
    covers = m_pCoverArtDAO->saveCoverArt(m_queueOfUpdates);
    m_pTrackDAO->updateCoverArt(covers);
    m_queueOfUpdates.clear();
}
