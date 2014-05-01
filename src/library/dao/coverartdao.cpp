#include <QImage>
#include <QtDebug>
#include <QThread>

#include "library/dao/coverartdao.h"

CoverArtDAO::CoverArtDAO(QSqlDatabase& database)
    : m_database(database) {
}

CoverArtDAO::~CoverArtDAO() {
}

void CoverArtDAO::initialize() {
    qDebug() << "CoverArtDAO::initialize"
             << QThread::currentThread()
             << m_database.connectionName();
}

void CoverArtDAO::saveCoverArt(TrackInfoObject* pTrack,
                               ConfigObject<ConfigValue>* pConfig) {

    QString settingsPath = pConfig->getSettingsPath();
    QString coverArtFolder = "/coverArt/";
    QDir dir(settingsPath.append(coverArtFolder));
    QDir().mkpath(dir.absolutePath());

    QString coverArtName = pTrack->getAlbum();
    QString location = dir.absolutePath().append("/").append(coverArtName);

    QImage image = pTrack->getCoverArt();
    if (!image.isNull())
        image.save(location, "JPG");
}
