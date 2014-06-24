#include <QDir>
#include <QPixmap>
#include <QStringBuilder>
#include <QtConcurrentRun>

#include "coverartcache.h"
#include "soundsourceproxy.h"

CoverArtCache::CoverArtCache()
        : m_pCoverArtDAO(NULL),
          m_pTrackDAO(NULL) {
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

    // trying find something directly on pixmapcache
    QString cover;
    if (coverLocation.isEmpty()) {
        cover = QString("embedded/%1").arg(trackId);
    } else {
        cover = coverLocation;
    }
    QPixmap pixmap;
    if (QPixmapCache::find(cover, &pixmap)) {
        emit(pixmapFound(trackId, pixmap));
        return;
    }

    QFuture<FutureResult> future;
    QFutureWatcher<FutureResult>* watcher = new QFutureWatcher<FutureResult>(this);
    if (coverLocation.isEmpty() || !QFile::exists(coverLocation)) {
        CoverArtDAO::CoverArtInfo coverInfo;
        coverInfo = m_pCoverArtDAO->getCoverArtInfo(trackId);
        // the coverLocation from tableview is updated just during the Mixxx loading,
        // it means that we could use the coverLocation from DB to try finding a pixmap.
        if (QPixmapCache::find(coverInfo.coverLocation, &pixmap)) {
            emit(pixmapFound(trackId, pixmap));
            return;
        }
        future = QtConcurrent::run(this, &CoverArtCache::searchImage, coverInfo);
        connect(watcher, SIGNAL(finished()), this, SLOT(imageFound()));
    } else {
        future = QtConcurrent::run(this, &CoverArtCache::loadImage,
                                   coverLocation, trackId);
        connect(watcher, SIGNAL(finished()), this, SLOT(imageLoaded()));
    }
    watcher->setFuture(future);
    m_runningIds.insert(trackId);
}

// Load cover from path stored in DB.
// It is executed in a separate thread via QtConcurrent::run
CoverArtCache::FutureResult CoverArtCache::loadImage(QString coverLocation,
                                                     int trackId) {
    FutureResult res;
    res.trackId = trackId;
    res.coverLocation = coverLocation;
    res.img = QImage(coverLocation);
    return res;
}

// watcher
void CoverArtCache::imageLoaded() {
    QFutureWatcher<FutureResult>* watcher;
    watcher = reinterpret_cast<QFutureWatcher<FutureResult>*>(sender());
    FutureResult res = watcher->result();

    if (!res.img.isNull()) {
        QPixmap pixmap = QPixmap::fromImage(res.img);
        if (QPixmapCache::insert(res.coverLocation, pixmap)) {
            emit(pixmapFound(res.trackId, pixmap));
        }
    }
    m_runningIds.remove(res.trackId);
}

// Searching and loading QImages is a very slow process
// that could block the main thread. Therefore, this method
// is executed in a separate thread via QtConcurrent::run
CoverArtCache::FutureResult CoverArtCache::searchImage(
        CoverArtDAO::CoverArtInfo coverInfo) {
    FutureResult res;
    res.trackId = coverInfo.trackId;

    // Looking for image with the same track name in the track dir.
    // (it allows that users have a cover per file)
    //
    // removing file extension
    coverInfo.trackFilename.remove(coverInfo.trackFilename.lastIndexOf("."),
                                   coverInfo.trackFilename.size() - 1);
    QStringList extList;
    extList << ".jpg" << ".jpeg" << ".png" << ".gif" << ".bmp";
    QString loc = coverInfo.trackDirectory % "/" % coverInfo.trackFilename;
    foreach (QString ext, extList) {
        res.img = QImage(loc + ext);
        if (!res.img.isNull()) {
            res.coverLocation = loc + ext;
            return res;
        }
    }

    // Looking for embedded cover art.
    //
    res.img = searchEmbeddedCover(coverInfo.trackLocation);
    if (!res.img.isNull()) {
        return res;
    }

    // Looking for cover stored in track diretory.
    //
    res.coverLocation = searchInTrackDirectory(coverInfo.trackDirectory,
                                               coverInfo.album);
    res.img = QImage(res.coverLocation);
    return res;
}

QString CoverArtCache::searchInTrackDirectory(QString directory, QString album) {
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
        return directory % "/" % imglist[0];
    }

    int idx;
    if (!album.isEmpty()) {
        idx  = imglist.indexOf(QRegExp(".*" % album % ".*", Qt::CaseInsensitive));
        if (idx  != -1 ) {
            return directory % "/" % imglist[idx];
        }
    }

    QList<QRegExp> regExpList;
    regExpList << QRegExp(".*cover.*", Qt::CaseInsensitive)
               << QRegExp(".*front.*", Qt::CaseInsensitive)
               << QRegExp(".*folder.*", Qt::CaseInsensitive);
    foreach (QRegExp regExp, regExpList) {
        idx  = imglist.indexOf(regExp);
        if (idx  != -1 ) {
            return directory % "/" % imglist[idx];
        }
    }

    return directory % "/" % imglist[0]; // lighter
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

// watcher
void CoverArtCache::imageFound() {
    QFutureWatcher<FutureResult>* watcher;
    watcher = reinterpret_cast<QFutureWatcher<FutureResult>*>(sender());
    FutureResult res = watcher->result();

    QString coverLocation;
    if (res.coverLocation.isEmpty()) {
        // we need a coverLocation to make the cache works (key)
        coverLocation = QString("embedded/%1").arg(res.trackId);
    } else {
        coverLocation = res.coverLocation;
        // update DB
        int coverId = m_pCoverArtDAO->saveCoverLocation(coverLocation);
        m_pTrackDAO->updateCoverArt(res.trackId, coverId);
    }

    if (!res.img.isNull()) {
        QPixmap pixmap = QPixmap::fromImage(res.img);
        if (QPixmapCache::insert(coverLocation, pixmap)) {
            emit(pixmapFound(res.trackId, pixmap));
        }
    }
    m_runningIds.remove(res.trackId);
}
