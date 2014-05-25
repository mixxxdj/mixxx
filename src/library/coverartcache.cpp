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
    if (m_keyHash.contains(location)) {
        QPixmap pixmap;
        if (QPixmapCache::find(m_keyHash.value(location), &pixmap)) {
            emit(responsePixmap(location, pixmap));
            return;
        } else {
            m_keyHash.remove(location);
        }
    }

    // load image from disk cache in a worker thread
    QFuture<coverPair> future = QtConcurrent::run(this,
                                                  &CoverArtCache::loadImage,
                                                  location);
    connect(&m_future_watcher, SIGNAL(finished()),
            this, SLOT(imageLoaded()));
    m_future_watcher.setFuture(future);
}

void CoverArtCache::imageLoaded() {
    coverPair p = m_future_watcher.result();
    QString location = p.first;
    QImage image = p.second;
    QPixmap pixmap;

    if (!image.isNull()) {
        pixmap = QPixmap::fromImage(image);
        QPixmapCache::Key key = QPixmapCache::insert(pixmap);
        m_keyHash.insert(location, key);
    }

    emit(responsePixmap(location, pixmap));
}

// This method is executed in a separate thread
// via QtConcurrent::run
CoverArtCache::coverPair CoverArtCache::loadImage(QString location) {
    return coverPair(location, QImage(location));
}
