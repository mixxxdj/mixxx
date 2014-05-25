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

void CoverArtCache::requestPixmap(QString location) {
    // keep a list of locations for which a future is currently running
    // to avoid loading the same picture again while we are loading it
    if (m_runningLocations.contains(location)) {
        return;
    }

    QPixmap pixmap;
    if (QPixmapCache::find(location, &pixmap)) {
        emit(pixmapFound(location, pixmap));
        return;
    }

    // load image from disk cache in a worker thread
    QFuture<coverPair> future = QtConcurrent::run(this,
                                                  &CoverArtCache::loadImage,
                                                  location);
    m_runningLocations.append(location);

    QFutureWatcher<coverPair>* watcher = new QFutureWatcher<coverPair>(this);
    connect(watcher, SIGNAL(finished()), this, SLOT(imageLoaded()));
    watcher->setFuture(future);
}

void CoverArtCache::imageLoaded() {
    QFutureWatcher<coverPair>* watcher;
    watcher = reinterpret_cast<QFutureWatcher<coverPair>*>(sender());

    coverPair p = watcher->result();
    QString location = p.first;
    QImage image = p.second;
    QPixmap pixmap;

    bool loaded = false;
    if (!image.isNull()) {
        pixmap = QPixmap::fromImage(image);
        if (QPixmapCache::insert(location, pixmap)) {
            loaded = true;
        }
    }

    if (loaded) {
        emit(pixmapFound(location, pixmap));
    } else {
        emit(pixmapNotFound(location));
    }

    m_runningLocations.removeOne(location);
}

// This method is executed in a separate thread
// via QtConcurrent::run
CoverArtCache::coverPair CoverArtCache::loadImage(QString location) {
    return coverPair(location, QImage(location));
}
