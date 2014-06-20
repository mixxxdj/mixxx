#include <QDir>
#include <QPixmap>
#include <QStringBuilder>
#include <QtConcurrentRun>

#include "coverartcache.h"
#include "soundsourceproxy.h"

CoverArtCache::CoverArtCache()
        : m_pCoverArtDAO(NULL) {
}

CoverArtCache::~CoverArtCache() {
}

void CoverArtCache::setCoverArtDAO(CoverArtDAO* coverdao) {
    m_pCoverArtDAO = coverdao;
}

void CoverArtCache::setTrackDAO(TrackDAO* trackdao) {
    m_pTrackDAO = trackdao;
}

void CoverArtCache::requestPixmap(QString coverLocation, int trackId) {
    if (trackId < 1) {
        return;
    }

    // keep a list of trackIds for which a future is currently running
    // to avoid loading the same picture again while we are loading it
    if (m_runningIds.contains(trackId)) {
        return;
    }

    CoverArtDAO::coverArtInfo coverInfo;
    if (QFile::exists(coverLocation)) {
        // it avoids doing db query to all pixmap requests.
        // if this location is valid, so likely the searchImage()
        // will find it direclty by the current location.
        coverInfo.trackId = trackId;
        coverInfo.currentCoverLocation = coverLocation;
    } else {
        coverInfo = m_pCoverArtDAO->getCoverArtInfo(trackId);
        coverLocation = coverInfo.currentCoverLocation;
    }

    QPixmap pixmap;
    if (QPixmapCache::find(coverLocation, &pixmap)) {
        emit(pixmapFound(trackId, pixmap));
        return;
    }

    // search and load image in a worker thread
    QFuture<threadRes> future = QtConcurrent::run(this,
                                                  &CoverArtCache::searchImage,
                                                  coverInfo);
    m_runningIds.insert(trackId);

    QFutureWatcher<threadRes>* watcher = new QFutureWatcher<threadRes>(this);
    connect(watcher, SIGNAL(finished()), this, SLOT(imageFound()));
    watcher->setFuture(future);
}

void CoverArtCache::imageFound() {
    QFutureWatcher<threadRes>* watcher;
    watcher = reinterpret_cast<QFutureWatcher<threadRes>*>(sender());
    threadRes res = watcher->result();

    if (!res.img.isNull()) {
        QPixmap pixmap = QPixmap::fromImage(res.img);
        if (QPixmapCache::insert(res.coverLocationFound, pixmap)) {
            emit(pixmapFound(res.trackId, pixmap));
        }
    }

    // checks if we have to update DB
    if (res.coverLocationFound != res.currentCoverLocation) {
        int coverId = m_pCoverArtDAO->saveCoverLocation(res.coverLocationFound);
        m_pTrackDAO->updateCoverArt(res.trackId, coverId);
    }

    m_runningIds.remove(res.trackId);
}

// Searching and loading QImages is a very slow process
// that could block the main thread. Therefore, this method
// is executed in a separate thread via QtConcurrent::run
CoverArtCache::threadRes CoverArtCache::searchImage(CoverArtDAO::coverArtInfo
                                                    coverInfo) {
    threadRes res;
    res.trackId = coverInfo.trackId;
    res.currentCoverLocation = coverInfo.currentCoverLocation;
    res.coverLocationFound = QString();
    res.img = QImage();

    // Looking for cover art in disk-cache directory.
    //
    QImage image = QImage(coverInfo.currentCoverLocation);
    if (!image.isNull()) {
        res.coverLocationFound = coverInfo.currentCoverLocation;
        res.img = image;
        return res;
    }
    image = QImage(coverInfo.defaultCoverLocation);
    if (!image.isNull()) {
        res.coverLocationFound = coverInfo.defaultCoverLocation;
        res.img = image;
        return res;
    }

    // Looking for embedded cover art and for cover stored in track diretory.
    //
    QImage newImage = searchEmbeddedCover(coverInfo.trackLocation);
    if (newImage.isNull()) {
        newImage = searchInTrackDirectory(coverInfo.trackDirectory);
    }
    if (saveImageInDisk(newImage, coverInfo.defaultCoverLocation)) {
        res.coverLocationFound = coverInfo.defaultCoverLocation;
        res.img = image;
        return res;
    }

    return res;
}

QImage CoverArtCache::searchInTrackDirectory(QString directory) {
    if (directory.isEmpty()) {
        return QImage();
    }

    QDir dir(directory);
    dir.setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Readable);
    dir.setSorting(QDir::Size | QDir::Reversed);

    QStringList nameFilters;
    nameFilters << "*.jpg" << "*.jpeg" << "*.png" << "*.gif" << "*.bmp";
    dir.setNameFilters(nameFilters);

    QString coverLocation;
    QStringList imglist = dir.entryList();
    if (imglist.size() > 0) {
        coverLocation = directory % "/" % imglist[0];
    }
    return QImage(coverLocation);
}

// this method will parse the information stored in the sound file
// just to extract the embedded cover art
QImage CoverArtCache::searchEmbeddedCover(QString trackLocation) {
    if (trackLocation.isEmpty()) {
        return QImage();
    }

    SecurityTokenPointer securityToken = Sandbox::openSecurityToken(
                                             QDir(trackLocation), true);
    SoundSourceProxy proxy(trackLocation, securityToken);
    Mixxx::SoundSource* pProxiedSoundSource = proxy.getProxiedSoundSource();
    if (pProxiedSoundSource != NULL && proxy.parseHeader() == OK) {
        return pProxiedSoundSource->getCoverArt();
    }
    return QImage();
}

bool CoverArtCache::saveImageInDisk(QImage cover, QString location) {
    if (cover.isNull()) {
        return false;
    }
    return cover.save(location, m_pCoverArtDAO->getDefaultImageFormat());
}
