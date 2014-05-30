#include <QFile>
#include <QMutex>
#include <QPixmap>
#include <QtConcurrentRun>

#include "coverartcache.h"

CoverArtCache* CoverArtCache::m_instance = NULL;
static QMutex s_Mutex;

CoverArtCache::CoverArtCache() {
}

CoverArtCache::~CoverArtCache() {
}

CoverArtCache* CoverArtCache::getInstance() {
    if (!m_instance) {
        s_Mutex.lock();
        if (!m_instance) {
            m_instance = new CoverArtCache();
        }
        s_Mutex.unlock();
    }
    return m_instance;
}

void CoverArtCache::destroyInstance() {
    s_Mutex.lock();
    if (m_instance) {
        delete m_instance;
        m_instance = 0;
    }
    s_Mutex.unlock();
}

void CoverArtCache::requestPixmap(TrackPointer pTrack) {
    if (pTrack.isNull()) {
        return;
    }

    QString coverLocation = pTrack->getCoverArtLocation();
    if (coverLocation.isEmpty()) {
        emit(pixmapNotFound(pTrack));
        return;
    }

    // keep a list of locations for which a future is currently running
    // to avoid loading the same picture again while we are loading it
    if (m_runningLocations.contains(coverLocation)) {
        return;
    }

    QPixmap pixmap;
    if (QPixmapCache::find(coverLocation, &pixmap)) {
        emit(pixmapFound(coverLocation, pixmap));
        return;
    }

    // load image from disk cache in a worker thread
    QFuture<coverPair> future = QtConcurrent::run(this,
                                                  &CoverArtCache::loadImage,
                                                  pTrack);
    m_runningLocations.append(coverLocation);

    QFutureWatcher<coverPair>* watcher = new QFutureWatcher<coverPair>(this);
    connect(watcher, SIGNAL(finished()), this, SLOT(imageLoaded()));
    watcher->setFuture(future);
}

void CoverArtCache::imageLoaded() {
    QFutureWatcher<coverPair>* watcher;
    watcher = reinterpret_cast<QFutureWatcher<coverPair>*>(sender());

    coverPair result = watcher->result();
    TrackPointer pTrack = result.first;
    QImage image = result.second;
    QString coverLocation = pTrack->getCoverArtLocation();
    QPixmap pixmap;

    bool loaded = false;
    if (!image.isNull()) {
        pixmap = QPixmap::fromImage(image);
        if (QPixmapCache::insert(coverLocation, pixmap)) {
            loaded = true;
        }
    }

    if (loaded) {
        emit(pixmapFound(coverLocation, pixmap));
    } else {
        emit(pixmapNotFound(pTrack));
    }

    m_runningLocations.removeOne(coverLocation);
}

// This method is executed in a separate thread
// via QtConcurrent::run
CoverArtCache::coverPair CoverArtCache::loadImage(TrackPointer pTrack) {
    return coverPair(pTrack, QImage(pTrack->getCoverArtLocation()));
}
