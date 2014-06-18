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

    // keep a list of trackIds for which a future is currently running
    // to avoid loading the same picture again while we are loading it
    if (m_runningIds.contains(trackId)) {
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
    m_runningIds.insert(trackId);

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
    if (!image.isNull()) {
        pixmap = QPixmap::fromImage(image);
        if (QPixmapCache::insert(coverLocation, pixmap)) {
            emit(pixmapFound(trackId, pixmap));
        }
    }
    m_runningIds.remove(trackId);
}

// This method is executed in a separate thread
// via QtConcurrent::run
CoverArtCache::coverTuple CoverArtCache::loadImage(QString coverLocation,
                                                   int trackId) {
    coverTuple result;
    result.trackId = trackId;
    result.coverLocation = coverLocation;
    result.img = QImage(coverLocation);
    if (!result.img.isNull()) {
        return result;
    }

    CoverArtDAO::coverArtInfo coverInfo = m_pCoverArtDAO->getCoverArtInfo(trackId);
    // looking for cover art in disk-cache directory.
    QString newCoverLocation = coverInfo.defaultCoverLocation;
    QImage newImage = QImage(newCoverLocation);
    if(newImage.isNull()) {
        // Looking for embedded cover art.
        newImage = searchEmbeddedCover(coverInfo.trackLocation);
        if (newImage.isNull()) {
            // Looking for cover stored in track diretory.
            newImage.load(searchInTrackDirectory(coverInfo.trackDirectory));
        }

        if (!saveImageInDisk(newImage, newCoverLocation)) {
            newCoverLocation.clear(); // not found
        }
    }
    result.img = newImage;

    if (coverLocation != newCoverLocation) {
        result.coverLocation = newCoverLocation;
        int coverId = m_pCoverArtDAO->saveCoverLocation(newCoverLocation);
        m_pCoverArtDAO->updateLibrary(trackId, coverId);
    }

    return result;
}

QString CoverArtCache::searchInTrackDirectory(QString directory) {
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
    return coverLocation;
}

// this method will parse the information stored in the sound file
// just to extract the embedded cover art
QImage CoverArtCache::searchEmbeddedCover(QString trackLocation) {
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
