#include <QPixmap>
#include <QtConcurrentRun>

#include "coverartcache.h"

CoverArtCache::CoverArtCache()
        : m_pCoverArtDAO(NULL) {
}

CoverArtCache::~CoverArtCache() {
}

void CoverArtCache::setCoverArtDao(CoverArtDAO* coverdao) {
    m_pCoverArtDAO = coverdao;
}

void CoverArtCache::requestPixmap(QString coverLocation, int trackId) {
    if (trackId < 1) {
        return;
    }

    if (coverLocation.isEmpty()) {
        coverLocation = m_pCoverArtDAO->getCoverArtLocation(trackId, true);
    }

    if (coverLocation.isEmpty()) {
        emit(pixmapNotFound(trackId));
        return;
    }

    // keep a list of locations for which a future is currently running
    // to avoid loading the same picture again while we are loading it
    if (m_runningLocations.contains(coverLocation)) {
        return;
    }

    QPixmap pixmap;
    if (QPixmapCache::find(coverLocation, &pixmap)) {
        emit(pixmapFound(trackId, pixmap));
        return;
    }

    // load image from disk cache in a worker thread
    QFuture<coverTuple> future = QtConcurrent::run(this,
                                                  &CoverArtCache::loadImage,
                                                  coverLocation, trackId);
    m_runningLocations.append(coverLocation);

    QFutureWatcher<coverTuple>* watcher = new QFutureWatcher<coverTuple>(this);
    connect(watcher, SIGNAL(finished()), this, SLOT(imageLoaded()));
    watcher->setFuture(future);
}

void CoverArtCache::imageLoaded() {
    QFutureWatcher<coverTuple>* watcher;
    watcher = reinterpret_cast<QFutureWatcher<coverTuple>*>(sender());

    coverTuple result = watcher->result();
    int trackId = result.trackId;
    QString coverLocation = result.coverLocation;
    QImage image = result.img;

    QPixmap pixmap;

    bool loaded = false;
    if (!image.isNull()) {
        pixmap = QPixmap::fromImage(image);
        if (QPixmapCache::insert(coverLocation, pixmap)) {
            loaded = true;
        }
    }

    if (loaded) {
        emit(pixmapFound(trackId, pixmap));
    } else {
        emit(pixmapNotFound(trackId));
    }

    m_runningLocations.removeOne(coverLocation);
}

// This method is executed in a separate thread
// via QtConcurrent::run
CoverArtCache::coverTuple CoverArtCache::loadImage(QString coverLocation,
                                                   int trackId) {
    coverTuple r;
    r.trackId = trackId;
    r.coverLocation = coverLocation;
    r.img = QImage(coverLocation);
    return r;
}
