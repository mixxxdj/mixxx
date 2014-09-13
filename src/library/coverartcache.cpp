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
          m_defaultCover(m_sDefaultCoverLocation) {
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

bool CoverArtCache::changeCoverArt(int trackId,
                                   const QString& newCoverLocation) {
    if (trackId < 1) {
        return false;
    }

    m_queueOfUpdates.remove(trackId);

    if (newCoverLocation.isEmpty()) {
        m_pTrackDAO->updateCoverArt(trackId, -1);
        return true;
    }

    QImage img(newCoverLocation);
    if (img.isNull()) {
        return false;
    }
    img = rescaleBigImage(img);
    QString md5Hash = calculateMD5(img);

    // Update DB
    int coverId = m_pCoverArtDAO->saveCoverArt(newCoverLocation, md5Hash);
    m_pTrackDAO->updateCoverArt(trackId, coverId);

    QPixmap pixmap;
    if (QPixmapCache::find(md5Hash, &pixmap)) {
        emit(pixmapFound(trackId, pixmap));
        emit(requestRepaint());
    } else {
        pixmap.convertFromImage(img);
        if (QPixmapCache::insert(md5Hash, pixmap)) {
            emit(pixmapFound(trackId, pixmap));
            emit(requestRepaint());
        }
    }

    return true;
}

QPixmap CoverArtCache::requestPixmap(CoverInfo info,
                                     const QSize& croppedSize,
                                     const bool onlyCached,
                                     const bool issueRepaint) {
    if (info.trackId < 1) {
        return QPixmap();
    }

    // keep a list of trackIds for which a future is currently running
    // to avoid loading the same picture again while we are loading it
    if (m_runningIds.contains(info.trackId)) {
        return QPixmap();
    }

    // check if we have already found a cover for this track
    // and if it is just waiting to be inserted/updated in the DB.
    if (m_queueOfUpdates.contains(info.trackId)) {
        info.md5Hash = m_queueOfUpdates[info.trackId].second;
    }

    // If this request comes from CoverDelegate (table view),
    // it'll want to get a cropped cover which is ready to be drawn
    // in the table view (cover art column).
    // It's very important to keep the cropped covers in cache because it avoids
    // having to rescale+crop it ALWAYS (which brings a lot of performance issues).
    QString cacheKey = QString("%1_%2x%3").arg(info.md5Hash)
                                          .arg(croppedSize.width())
                                          .arg(croppedSize.height());
    QPixmap pixmap;
    if (QPixmapCache::find(cacheKey, &pixmap)) {
        if (!issueRepaint) {
            emit(pixmapFound(info.trackId, pixmap));
        }
        return pixmap;
    }

    if (onlyCached) {
        return QPixmap();
    }

    QFuture<FutureResult> future;
    QFutureWatcher<FutureResult>* watcher = new QFutureWatcher<FutureResult>(this);
    CoverArtDAO::CoverArtInfo coverInfo;
    // (bool != bool) is equivalent to a logical XOR
    if (info.md5Hash.isEmpty() ||
           ((info.coverLocation == "ID3TAG") != !QFile::exists(info.coverLocation))) {
        coverInfo = m_pCoverArtDAO->getCoverArtInfo(info.trackId);
        future = QtConcurrent::run(this, &CoverArtCache::searchImage,
                                   coverInfo, croppedSize, issueRepaint);
        connect(watcher, SIGNAL(finished()), this, SLOT(imageFound()));
    } else {
        coverInfo.trackId = info.trackId;
        coverInfo.coverLocation = info.coverLocation;
        coverInfo.md5Hash = info.md5Hash;
        coverInfo.trackLocation = info.trackLocation;
        future = QtConcurrent::run(this, &CoverArtCache::loadImage,
                                   coverInfo, croppedSize, issueRepaint);
        connect(watcher, SIGNAL(finished()), this, SLOT(imageLoaded()));
    }
    m_runningIds.insert(info.trackId);
    watcher->setFuture(future);

    return QPixmap();
}

// Load cover from path stored in DB.
// It is executed in a separate thread via QtConcurrent::run
CoverArtCache::FutureResult CoverArtCache::loadImage(CoverArtDAO::CoverArtInfo coverInfo,
                                                     const QSize& croppedSize,
                                                     const bool issueRepaint) {
    FutureResult res;
    res.trackId = coverInfo.trackId;
    res.coverLocation = coverInfo.coverLocation;
    res.md5Hash = coverInfo.md5Hash;
    res.croppedSize = croppedSize;
    res.issueRepaint = issueRepaint;

    if (res.coverLocation == "ID3TAG") {
        res.img = extractEmbeddedCover(coverInfo.trackLocation);
    } else {
        res.img = QImage(res.coverLocation);
    }

    if (res.croppedSize.isNull()) {
        res.img = rescaleBigImage(res.img);
    } else {
        res.img = cropImage(res.img, res.croppedSize);
    }

    return res;
}

// watcher
void CoverArtCache::imageLoaded() {
    QFutureWatcher<FutureResult>* watcher;
    watcher = reinterpret_cast<QFutureWatcher<FutureResult>*>(sender());
    FutureResult res = watcher->result();

    QString cacheKey = QString("%1_%2x%3").arg(res.md5Hash)
                                          .arg(res.croppedSize.width())
                                          .arg(res.croppedSize.height());

    QPixmap pixmap;
    QPixmapCache::find(cacheKey, &pixmap);
    if (pixmap.isNull() && !res.img.isNull()) {
        pixmap.convertFromImage(res.img);
        QPixmapCache::insert(cacheKey, pixmap);
    }

    // The widgets expects a signal response.
    if (pixmap.isNull()) {
        pixmap = m_defaultCover;
    }

    if (res.issueRepaint) {
        emit(requestRepaint());
    } else {
        emit(pixmapFound(res.trackId, pixmap));
    }

    m_runningIds.remove(res.trackId);
}

// Searching and loading QImages is a very slow process
// that could block the main thread. Therefore, this method
// is executed in a separate thread via QtConcurrent::run
CoverArtCache::FutureResult CoverArtCache::searchImage(
                                           CoverArtDAO::CoverArtInfo coverInfo,
                                           const QSize& croppedSize,
                                           const bool issueRepaint) {
    FutureResult res;
    res.trackId = coverInfo.trackId;
    res.croppedSize = croppedSize;
    res.issueRepaint = issueRepaint;
    bool newImgFound = false;

    // Looking for embedded cover art.
    //
    res.img = extractEmbeddedCover(coverInfo.trackLocation);
    if (!res.img.isNull()) {
        res.img = rescaleBigImage(res.img);
        res.md5Hash = calculateMD5(res.img);
        res.coverLocation = "ID3TAG";
        newImgFound = true;
    }

    // Looking for cover stored in track diretory.
    //
    if (!newImgFound) {
        res.coverLocation = searchInTrackDirectory(coverInfo.trackDirectory,
                                                   coverInfo.trackBaseName,
                                                   coverInfo.album);
        res.img = rescaleBigImage(QImage(res.coverLocation));
        res.md5Hash = calculateMD5(res.img);
        newImgFound = true;
    }

    // adjusting the cover size according to the final purpose
    if (newImgFound && !res.croppedSize.isNull()) {
        res.img = cropImage(res.img, res.croppedSize);
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

    QString cacheKey = QString("%1_%2x%3").arg(res.md5Hash)
                                          .arg(res.croppedSize.width())
                                          .arg(res.croppedSize.height());

    QPixmap pixmap;
    QPixmapCache::find(cacheKey, &pixmap);
    if (pixmap.isNull() && !res.img.isNull()) {
        pixmap.convertFromImage(res.img);
        QPixmapCache::insert(cacheKey, pixmap);
    }

    // The widgets expects a signal response.
    if (pixmap.isNull()) {
        pixmap = m_defaultCover;
    }

    if (res.issueRepaint) {
        emit(requestRepaint());
    } else {
        emit(pixmapFound(res.trackId, pixmap));
    }

    // update DB
    if (!m_queueOfUpdates.contains(res.trackId)) {
        m_queueOfUpdates.insert(res.trackId,
                                qMakePair(res.coverLocation, res.md5Hash));
        updateDB();
    }

    m_runningIds.remove(res.trackId);
}

// sqlite can't do a huge number of updates in a very short time,
// so it is important to collect all new covers and write them at once.
void CoverArtCache::updateDB(bool forceUpdate) {
    if (!forceUpdate && m_queueOfUpdates.size() < 500) {
        return;
    }

    qDebug() << "CoverCache : Updating" << m_queueOfUpdates.size() << "tracks!";

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
QImage CoverArtCache::cropImage(QImage img, const QSize& finalSize) {
    if (img.isNull()) {
        return QImage();
    }
    img = img.scaledToWidth(finalSize.width(), Qt::SmoothTransformation);
    return img.copy(0, 0, img.width(), finalSize.height());
}

// if it's too big, we have to scale it.
// big images would be quickly removed from cover cache.
QImage CoverArtCache::rescaleBigImage(QImage img) {
    const int kMaxSize = 300;
    QSize size = img.size();
    if (size.height() > kMaxSize || size.width() > kMaxSize) {
        return img.scaled(kMaxSize, kMaxSize,
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
