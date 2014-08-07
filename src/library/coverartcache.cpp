#include <QCryptographicHash>
#include <QDir>
#include <QPixmap>
#include <QStringBuilder>
#include <QtConcurrentRun>
#include <QTimer>

#include "coverartcache.h"
#include "soundsourceproxy.h"

CoverArtCache::CoverArtCache()
        : m_pCoverArtDAO(NULL),
          m_pTrackDAO(NULL),
          m_sDefaultCoverLocation(":/images/library/default_cover.png"),
          m_defaultCover(m_sDefaultCoverLocation),
          m_timer(new QTimer(this)) {
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), SLOT(updateDB()));
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

    QImage img = rescaleBigImage(QImage(newCoverLocation));
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

QPixmap CoverArtCache::requestPixmap(int trackId,
                                     const QString& coverLocation,
                                     const QString& md5Hash,
                                     const bool tryLoadAndSearch,
                                     const bool croppedPixmap,
                                     const bool fromDelegate) {
    if (trackId < 1) {
        return QPixmap();
    }

    // keep a list of trackIds for which a future is currently running
    // to avoid loading the same picture again while we are loading it
    if (m_runningIds.contains(trackId)) {
        return QPixmap();
    }

    // check if we have already found a cover for this track
    // and if it is just waiting to be inserted/updated in the DB.
    if (m_queueOfUpdates.contains(trackId)) {
        return QPixmap();
    }

    // If this request comes from CoverDelegate (table view),
    // it'll want to get a cropped cover which is ready to be drawn
    // in the table view (cover art column).
    // It's very important to keep the cropped covers in cache because it avoids
    // having to rescale+crop it ALWAYS (which brings a lot of performance issues).
    QString cacheKey = croppedPixmap ? md5Hash % "_cropped" : md5Hash;

    QPixmap pixmap;
    if (QPixmapCache::find(cacheKey, &pixmap)) {
        if (fromDelegate) {
            emit(requestRepaint());
        } else {
            emit(pixmapFound(trackId, pixmap));
        }
        return pixmap;
    }

    if (!tryLoadAndSearch) {
        return QPixmap();
    }

    QFuture<FutureResult> future;
    QFutureWatcher<FutureResult>* watcher = new QFutureWatcher<FutureResult>(this);
    if (coverLocation.isEmpty() || !QFile::exists(coverLocation)) {
        CoverArtDAO::CoverArtInfo coverInfo;
        coverInfo = m_pCoverArtDAO->getCoverArtInfo(trackId);
        future = QtConcurrent::run(this, &CoverArtCache::searchImage,
                                   coverInfo, croppedPixmap, fromDelegate);
        connect(watcher, SIGNAL(finished()), this, SLOT(imageFound()));
    } else {
        future = QtConcurrent::run(this, &CoverArtCache::loadImage,
                                   trackId, coverLocation, md5Hash,
                                   croppedPixmap, fromDelegate);
        connect(watcher, SIGNAL(finished()), this, SLOT(imageLoaded()));
    }
    m_runningIds.insert(trackId);
    watcher->setFuture(future);

    return QPixmap();
}

// Load cover from path stored in DB.
// It is executed in a separate thread via QtConcurrent::run
CoverArtCache::FutureResult CoverArtCache::loadImage(int trackId,
                                                     const QString& coverLocation,
                                                     const QString& md5Hash,
                                                     const bool croppedPixmap,
                                                     const bool fromDelegate) {
    FutureResult res;
    res.trackId = trackId;
    res.coverLocation = coverLocation;
    res.img = QImage(coverLocation);
    res.md5Hash = md5Hash;
    res.croppedImg = croppedPixmap;
    res.fromDelegate = fromDelegate;
    res.newImgFound = true;

    if (res.croppedImg) {
        res.img = cropImage(res.img);
    } else {
        res.img = rescaleBigImage(res.img);
    }

    return res;
}

// watcher
void CoverArtCache::imageLoaded() {
    QFutureWatcher<FutureResult>* watcher;
    watcher = reinterpret_cast<QFutureWatcher<FutureResult>*>(sender());
    FutureResult res = watcher->result();

    QPixmap pixmap;
    QString cacheKey = res.croppedImg ? res.md5Hash % "_cropped" : res.md5Hash;
    if (QPixmapCache::find(cacheKey, &pixmap) && res.fromDelegate) {
        emit(pixmapFound(res.trackId, pixmap));
    } else if (!res.img.isNull()) {
        pixmap.convertFromImage(res.img);
        if (QPixmapCache::insert(cacheKey, pixmap)) {
            if (res.fromDelegate) {
                emit(requestRepaint());
            } else {
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
                                           CoverArtDAO::CoverArtInfo coverInfo,
                                           const bool croppedPixmap,
                                           const bool fromDelegate) {
    FutureResult res;
    res.trackId = coverInfo.trackId;
    res.md5Hash = coverInfo.md5Hash;
    res.croppedImg = croppedPixmap;
    res.fromDelegate = fromDelegate;
    res.newImgFound = false;

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
        res.newImgFound = true;
    }

    // Looking for cover stored in track diretory.
    //
    if (!res.newImgFound) {
        res.coverLocation = searchInTrackDirectory(coverInfo.trackDirectory,
                                                   coverInfo.trackBaseName,
                                                   coverInfo.album);
        res.img = rescaleBigImage(QImage(res.coverLocation));
        res.md5Hash = calculateMD5(res.img);
        res.newImgFound = true;
    }

    // adjusting the cover size according to the final purpose
    if (res.newImgFound && res.croppedImg) {
        res.img = cropImage(res.img);
    }

    // check if this image is really a new one
    // (different from the one that we have in db)
    if (coverInfo.md5Hash == res.md5Hash)
    {
        res.newImgFound = false;
    }

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
    QString cacheKey = res.croppedImg ? res.md5Hash % "_cropped" : res.md5Hash;
    if (QPixmapCache::find(cacheKey, &pixmap) && res.fromDelegate) {
        emit(pixmapFound(res.trackId, pixmap));
    } else if (!res.img.isNull()) {
        pixmap.convertFromImage(res.img);
        if (QPixmapCache::insert(cacheKey, pixmap)) {
            if (res.fromDelegate) {
                emit(requestRepaint());
            } else {
                emit(pixmapFound(res.trackId, pixmap));
            }
        }
    }

    // update DB
    if (res.newImgFound && !m_queueOfUpdates.contains(res.trackId)) {
        m_queueOfUpdates.insert(res.trackId,
                                qMakePair(res.coverLocation, res.md5Hash));
    }

    if (m_queueOfUpdates.size() == 1 && !m_timer->isActive()) {
        m_timer->start(500); // after 0.5s, it will call `updateDB()`
    }

    m_runningIds.remove(res.trackId);
}

// sqlite can't do a huge number of updates in a very short time,
// so it is important to collect all new covers and write them at once.
void CoverArtCache::updateDB() {
    if (m_queueOfUpdates.isEmpty()) {
        return;
    }
    QSet<QPair<int, int> > covers;
    covers = m_pCoverArtDAO->saveCoverArt(m_queueOfUpdates);
    m_pTrackDAO->updateCoverArt(covers);
    m_queueOfUpdates.clear();
}

// It will return a cropped cover that is ready to be
// used by the tableview-cover_column (CoverDelegate).
// As QImage is optimized to manipulate images, we will do it here
// instead of rescale it directly on the CoverDelegate::paint()
// because it would be much slower and could easily freeze the UI...
// Also, this method will run in separate thread
// (via Qtconcurrent - called by searchImage() or loadImage())
QImage CoverArtCache::cropImage(QImage img) {
    if (img.isNull()) {
        return QImage();
    }

    // it defines the maximum width of the covers displayed
    // in the cover art column (tableviews).
    // (if you want to increase it - you have to change JUST it.)
    const int WIDTH = 100;
    const int CELL_HEIGHT = 20;

    img = img.scaledToWidth(WIDTH, Qt::SmoothTransformation);
    return img.copy(0, 0, img.width(), CELL_HEIGHT);
}

// if it's too big, we have to scale it.
// big images would be quickly removed from cover cache.
QImage CoverArtCache::rescaleBigImage(QImage img) {
    const int MAXSIZE = 300;
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
