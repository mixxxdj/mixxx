#include <QtCore>
#include <QtGui>
#include <QtSql>
#include <QtDebug>

#include "trackcollection.h"

#include "defs.h"
#include "library/librarytablemodel.h"
#include "library/schemamanager.h"
#include "soundsourceproxy.h"
#include "trackinfoobject.h"
#include "xmlparse.h"

TrackCollection::TrackCollection(ConfigObject<ConfigValue>* pConfig)
        : m_pConfig(pConfig),
          m_db(QSqlDatabase::addDatabase("QSQLITE")), // defaultConnection
          m_playlistDao(m_db),
          m_crateDao(m_db),
          m_cueDao(m_db),
          m_analysisDao(m_db),
          m_trackDao(m_db, m_cueDao, m_playlistDao, m_crateDao, m_analysisDao, pConfig),
          m_supportedFileExtensionsRegex(
              SoundSourceProxy::supportedFileExtensionsRegex(),
              Qt::CaseInsensitive) {
    bCancelLibraryScan = false;
    qDebug() << "Available QtSQL drivers:" << QSqlDatabase::drivers();

    m_db.setHostName("localhost");
    m_db.setDatabaseName(MIXXX_DB_PATH);
    m_db.setUserName("mixxx");
    m_db.setPassword("mixxx");
    bool ok = m_db.open();
    qDebug() << __FILE__ << "DB status:" << ok;
    if (m_db.lastError().isValid()) {
        qDebug() << "Error loading database:" << m_db.lastError();
    }
    // Check for tables and create them if missing
    if (!checkForTables()) {
        // TODO(XXX) something a little more elegant
        exit(-1);
    }
}

TrackCollection::~TrackCollection() {
    qDebug() << "~TrackCollection()";
    m_trackDao.finish();

    if (m_db.isOpen()) {
        // There should never be an outstanding transaction when this code is
        // called. If there is, it means we probably aren't committing a
        // transaction somewhere that should be.
        if (m_db.rollback()) {
            qDebug() << "ERROR: There was a transaction in progress on the main database connection while shutting down."
                     << "There is a logic error somewhere.";
        }
        m_db.close();
    } else {
        qDebug() << "ERROR: The main database connection was closed before TrackCollection closed it."
                 << "There is a logic error somewhere.";
    }
}

bool TrackCollection::checkForTables() {
    if (!m_db.open()) {
        QMessageBox::critical(0, tr("Cannot open database"),
                              tr("Unable to establish a database connection.\n"
                                 "Mixxx requires QT with SQLite support. Please read "
                                 "the Qt SQL driver documentation for information on how "
                                 "to build it.\n\n"
                                 "Click OK to exit."), QMessageBox::Ok);
        return false;
    }

    int requiredSchemaVersion = 18;
    if (!SchemaManager::upgradeToSchemaVersion(m_pConfig, m_db,
                                               requiredSchemaVersion)) {
        QMessageBox::warning(0, tr("Cannot upgrade database schema"),
                             tr("Unable to upgrade your database schema to version %1.\n"
                                "Your mixxx.db file may be corrupt.\n"
                                "Try renaming it and restarting Mixxx.\n\n"
                                "Click OK to exit.").arg(requiredSchemaVersion),
                             QMessageBox::Ok);
        return false;
    }

    m_trackDao.initialize();
    m_playlistDao.initialize();
    m_crateDao.initialize();
    m_cueDao.initialize();

    return true;
}

QSqlDatabase& TrackCollection::getDatabase() {
    return m_db;
}

/** Do a non-recursive import of all the songs in a directory. Does NOT decend into subdirectories.
    @param trackDao The track data access object which provides a connection to the database. We use this parameter in order to make this function callable from separate threads. You need to use a different DB connection for each thread.
    @return true if the scan completed without being cancelled. False if the scan was cancelled part-way through.
*/
bool TrackCollection::importDirectory(QString directory, TrackDAO &trackDao,
                                      QList<TrackInfoObject*>& tracksToAdd) {
    //qDebug() << "TrackCollection::importDirectory(" << directory<< ")";

    emit(startedLoading());
    QFileInfoList files;

    //Check to make sure the path exists.
    QDir dir(directory);
    if (dir.exists()) {
        files = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    } else {
        qDebug() << "Error: Import path does not exist." << directory;
        return true;
    }

    //The directory exists, so get a list of the contents of the directory and go through it.
    QList<QFileInfo>::iterator it = files.begin();
    while (it != files.end()) {
        QFileInfo file = *it; //TODO: THIS IS SLOW!
        it++;

        //If a flag was raised telling us to cancel the library scan then stop.
        m_libraryScanMutex.lock();
        bool cancel = bCancelLibraryScan;
        m_libraryScanMutex.unlock();
        if (cancel) {
            return false;
        }

        QString absoluteFilePath = file.absoluteFilePath();
        QString fileName = file.fileName();

        if (fileName.count(m_supportedFileExtensionsRegex)) {
            //If the track is in the database, mark it as existing. This code gets exectuted
            //when other files in the same directory have changed (the directory hash has changed).
            trackDao.markTrackLocationAsVerified(absoluteFilePath);

            // If the file already exists in the database, continue and go on to
            // the next file.

            // If the file doesn't already exist in the database, then add
            // it. If it does exist in the database, then it is either in the
            // user's library OR the user has "removed" the track via
            // "Right-Click -> Remove". These tracks stay in the library, but
            // their mixxx_deleted column is 1.
            if (!trackDao.trackExistsInDatabase(absoluteFilePath))
            {
                //qDebug() << "Loading" << file.fileName();
                emit(progressLoading(fileName));

                // addTrack uses this QFileInfo instead of making a new one now.
                //trackDao.addTrack(file);
                TrackInfoObject* pTrack = new TrackInfoObject(file.absoluteFilePath());
                tracksToAdd.append(pTrack);
            }
        } else {
            //qDebug() << "Skipping" << file.fileName() <<
            //    "because it did not match thesupported audio files filter:" <<
        }
    }
    emit(finishedLoading());
    return true;
}

void TrackCollection::slotCancelLibraryScan() {
    m_libraryScanMutex.lock();
    bCancelLibraryScan = 1;
    m_libraryScanMutex.unlock();
}

void TrackCollection::resetLibaryCancellation() {
    m_libraryScanMutex.lock();
    bCancelLibraryScan = 0;
    m_libraryScanMutex.unlock();
}

CrateDAO& TrackCollection::getCrateDAO() {
    return m_crateDao;
}

TrackDAO& TrackCollection::getTrackDAO() {
    return m_trackDao;
}

PlaylistDAO& TrackCollection::getPlaylistDAO() {
    return m_playlistDao;
}

QSharedPointer<BaseTrackCache> TrackCollection::getTrackSource(
    const QString name) {
    return m_trackSources.value(name, QSharedPointer<BaseTrackCache>());
}

void TrackCollection::addTrackSource(
    const QString name, QSharedPointer<BaseTrackCache> trackSource) {
    Q_ASSERT(!m_trackSources.contains(name));
    m_trackSources[name] = trackSource;
}
