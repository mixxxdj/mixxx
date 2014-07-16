#include <QCryptographicHash>
#include <QDir>
#include <QPixmap>
#include <QStringBuilder>
#include <QtConcurrentRun>

#include "coverartcache.h"
#include "soundsourceproxy.h"

CoverArtCache::CoverArtCache()
        : m_pCoverArtDAO(NULL),
          m_pTrackDAO(NULL),
          m_sDefaultCoverLocation(":/images/library/default_cover.png"),
          m_defaultCover(m_sDefaultCoverLocation) {
}

CoverArtCache::~CoverArtCache() {
}

void CoverArtCache::setCoverArtDAO(CoverArtDAO* coverdao) {
    m_pCoverArtDAO = coverdao;
}

void CoverArtCache::setTrackDAO(TrackDAO* trackdao) {
    m_pTrackDAO = trackdao;
}

bool CoverArtCache::changeCoverArt(int trackId,
                                   const QString& newCoverLocation) {
    if (trackId < 1 || newCoverLocation.isEmpty()) {
        return false;
    }

    QImage img(newCoverLocation);
    QString md5Hash = calculateMD5(img);
    if (md5Hash.isEmpty()) {
        return false;
    }

    // Update DB
    int coverId = m_pCoverArtDAO->saveCoverArt(newCoverLocation, md5Hash);
    m_pTrackDAO->updateCoverArt(trackId, coverId);

    QPixmap pixmap;
    if (QPixmapCache::find(md5Hash, &pixmap)) {
        emit(pixmapFound(trackId, pixmap));
    } else {
        pixmap.convertFromImage(img);
        if (QPixmapCache::insert(md5Hash, pixmap)) {
            emit(pixmapFound(trackId, pixmap));
        }
    }

    return true;
}

QString CoverArtCache::getHashOfEmbeddedCover(QString trackLocation) {
    QImage img = extractEmbeddedCover(trackLocation);
    QString md5Hash;
    if (!img.isNull()) {
        img = rescaleBigImage(img);
        md5Hash = calculateMD5(img);
    }
    return md5Hash;
}

QPixmap CoverArtCache::requestPixmap(int trackId,
                                     const QString& coverLocation,
                                     const QString& md5Hash,
                                     const bool emitSignals) {
    if (trackId < 1) {
        return m_defaultCover;
    }

    // keep a list of trackIds for which a future is currently running
    // to avoid loading the same picture again while we are loading it
    if (m_runningIds.contains(trackId)) {
        return m_defaultCover;
    }

    QPixmap pixmap;
    if (QPixmapCache::find(md5Hash, &pixmap)) {
        if (emitSignals) {
            emit(pixmapFound(trackId, pixmap));
        }
        return pixmap;
    }

    QFuture<FutureResult> future;
    QFutureWatcher<FutureResult>* watcher = new QFutureWatcher<FutureResult>(this);
    if (coverLocation.isEmpty() || !QFile::exists(coverLocation)) {
        CoverArtDAO::CoverArtInfo coverInfo;
        coverInfo = m_pCoverArtDAO->getCoverArtInfo(trackId);
        future = QtConcurrent::run(this, &CoverArtCache::searchImage,
                                   coverInfo, emitSignals);
        connect(watcher, SIGNAL(finished()), this, SLOT(imageFound()));
    } else {
        future = QtConcurrent::run(this, &CoverArtCache::loadImage,
                                   trackId, coverLocation, md5Hash, emitSignals);
        connect(watcher, SIGNAL(finished()), this, SLOT(imageLoaded()));
    }
    m_runningIds.insert(trackId);
    watcher->setFuture(future);

    return m_defaultCover;
}

// Load cover from path stored in DB.
// It is executed in a separate thread via QtConcurrent::run
CoverArtCache::FutureResult CoverArtCache::loadImage(int trackId,
                                                     const QString& coverLocation,
                                                     const QString& md5Hash,
                                                     const bool emitSignals) {
    FutureResult res;
    res.trackId = trackId;
    res.coverLocation = coverLocation;
    res.img = QImage(coverLocation);
    res.img = rescaleBigImage(res.img);
    res.md5Hash = md5Hash;
    res.emitSignals = emitSignals;
    return res;
}

// watcher
void CoverArtCache::imageLoaded() {
    QFutureWatcher<FutureResult>* watcher;
    watcher = reinterpret_cast<QFutureWatcher<FutureResult>*>(sender());
    FutureResult res = watcher->result();

    QPixmap pixmap;
    if (QPixmapCache::find(res.md5Hash, &pixmap) && res.emitSignals) {
        emit(pixmapFound(res.trackId, pixmap));
    } else if (!res.img.isNull()) {
        pixmap.convertFromImage(res.img);
        if (QPixmapCache::insert(res.md5Hash, pixmap)) {
            if (res.emitSignals) {
                emit(pixmapFound(res.trackId, pixmap));
            }
        }
    }
    m_runningIds.remove(res.trackId);
}

// Searching and loading QImages is a very slow process
// that could block the main thread. Therefore, this method
// is executed in a separate thread via QtConcurrent::run
CoverArtCache::FutureResult CoverArtCache::searchImage(
        CoverArtDAO::CoverArtInfo coverInfo, const bool emitSignals) {
    FutureResult res;
    res.trackId = coverInfo.trackId;
    res.emitSignals = emitSignals;

    // Looking for embedded cover art.
    //
    res.img = extractEmbeddedCover(coverInfo.trackLocation);
    if (!res.img.isNull()) {
        res.img = rescaleBigImage(res.img);
        if (res.md5Hash.isEmpty()) {
            // it is the first time that we are loading the embedded cover,
            // so we need to recalculate the md5 hash.
            res.md5Hash = calculateMD5(res.img);
        }
        return res;
    }

    // Looking for cover stored in track diretory.
    //
    res.coverLocation = searchInTrackDirectory(coverInfo.trackDirectory,
                                               coverInfo.trackBaseName,
                                               coverInfo.album);
    res.img = QImage(res.coverLocation);
    res.img = rescaleBigImage(res.img);
    res.md5Hash = calculateMD5(res.img);
    return res;
}

QString CoverArtCache::searchInTrackDirectory(QString directory,
                                              QString trackBaseName,
                                              QString album) {
    if (directory.isEmpty()) {
        return QString();
    }

    QDir dir(directory);
    dir.setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Readable);
    dir.setSorting(QDir::Size | QDir::Reversed);

    QStringList nameFilters;
    nameFilters << "*.jpg" << "*.jpeg" << "*.png" << "*.gif" << "*.bmp";
    dir.setNameFilters(nameFilters);

    QStringList imglist = dir.entryList();
    if (imglist.size() < 1) {
        return QString();
    } else if (imglist.size() == 1) {
        // only a single picture in folder.
        return directory % "/" % imglist[0];
    }

    int idx  = imglist.indexOf(QRegExp(".*" % trackBaseName % ".*",
                                       Qt::CaseInsensitive));
    if (idx  != -1 ) {
        // cover with the same name of the trackFilename.
        return directory % "/" % imglist[idx];
    }

    if (!album.isEmpty()) {
        idx  = imglist.indexOf(QRegExp(".*" % album % ".*",
                                       Qt::CaseInsensitive));
        if (idx  != -1 ) {
            // cover with the same name of the album.
            return directory % "/" % imglist[idx];
        }
    }

    QList<QRegExp> regExpList;
    regExpList << QRegExp(".*cover.*", Qt::CaseInsensitive)
               << QRegExp(".*front.*", Qt::CaseInsensitive)
               << QRegExp(".*album.*", Qt::CaseInsensitive)
               << QRegExp(".*folder.*", Qt::CaseInsensitive);
    foreach (QRegExp regExp, regExpList) {
        idx  = imglist.indexOf(regExp);
        if (idx  != -1 ) {
            // cover named as cover|front|folder.
            return directory % "/" % imglist[idx];
        }
    }

    // Return the lighter image file.
    return directory % "/" % imglist[0];
}

// this method will parse the information stored in the sound file
// just to extract the embedded cover art
QImage CoverArtCache::extractEmbeddedCover(QString trackLocation) {
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

// watcher
void CoverArtCache::imageFound() {
    QFutureWatcher<FutureResult>* watcher;
    watcher = reinterpret_cast<QFutureWatcher<FutureResult>*>(sender());
    FutureResult res = watcher->result();

    QPixmap pixmap;
    if (QPixmapCache::find(res.md5Hash, &pixmap) && res.emitSignals) {
        emit(pixmapFound(res.trackId, pixmap));
    } else if (!res.img.isNull()) {
        pixmap.convertFromImage(res.img);
        if (QPixmapCache::insert(res.md5Hash, pixmap)) {
            if (res.emitSignals) {
                emit(pixmapFound(res.trackId, pixmap));
            }
        }
    }
    // update DB
    int coverId = m_pCoverArtDAO->saveCoverArt(res.coverLocation, res.md5Hash);
    m_pTrackDAO->updateCoverArt(res.trackId, coverId);

    m_runningIds.remove(res.trackId);
}

// if it's too big, we have to scale it.
// big images would be quickly removed from cover cache.
QImage CoverArtCache::rescaleBigImage(QImage img) {
    const int MAXSIZE = 400;
    QSize size = img.size();
    if (size.height() > MAXSIZE || size.width() > MAXSIZE) {
        return img.scaled(MAXSIZE, MAXSIZE,
                          Qt::KeepAspectRatio,
                          Qt::SmoothTransformation);
    } else {
        return img;
    }
}

QString CoverArtCache::calculateMD5(QImage img) {
    if (img.isNull()) {
        return QString();
    }
    QByteArray arr((char*)img.bits(), img.byteCount());
    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(arr);
    return md5.result().toHex();
}
