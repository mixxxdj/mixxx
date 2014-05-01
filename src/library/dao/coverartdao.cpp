#include <QImage>
#include <QtDebug>
#include <QThread>

#include "library/dao/coverartdao.h"

CoverArtDAO::CoverArtDAO(QSqlDatabase& database, ConfigObject<ConfigValue>* pConfig)
        : m_pConfig(pConfig),
          m_database(database) {
    if (!QDir().mkpath(getStoragePath())) {
        qDebug() << "WARNING: Could not create cover arts storage path. "
                 << "Mixxx will be unable to store analyses.";
    }
}

CoverArtDAO::~CoverArtDAO() {
}

void CoverArtDAO::initialize() {
    qDebug() << "CoverArtDAO::initialize"
             << QThread::currentThread()
             << m_database.connectionName();
}

void CoverArtDAO::saveCoverArt(TrackInfoObject* pTrack) {
    QString coverArtName = pTrack->getAlbum();
    QImage image = pTrack->getCoverArt();
    if (!image.isNull())
        image.save(getStoragePath().append(coverArtName), "JPG");
}

QString CoverArtDAO::getStoragePath() const {
    QString settingsPath = m_pConfig->getSettingsPath();
    QDir dir(settingsPath.append("/coverArt/"));
    return dir.absolutePath().append("/");
}
