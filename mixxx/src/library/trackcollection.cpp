#include <QtCore>
#include <QtGui>
#include <QtSql>
#include <QtDebug>

#include "trackcollection.h"
#include "xmlparse.h"
#include "trackinfoobject.h"
#include "defs.h"
#include "soundsourceproxy.h"
#include "library/schemamanager.h"

TrackCollection::TrackCollection(ConfigObject<ConfigValue>* pConfig)
        : m_pConfig(pConfig),
          m_db(QSqlDatabase::addDatabase("QSQLITE")),
          m_playlistDao(m_db),
          m_cueDao(m_db),
          m_trackDao(m_db, m_cueDao),
          m_crateDao(m_db) {

    bCancelLibraryScan = 0;
    qDebug() << QSqlDatabase::drivers();

    m_db.setHostName("localhost");
    m_db.setDatabaseName(MIXXX_DB_PATH);
    m_db.setUserName("mixxx");
    m_db.setPassword("mixxx");
    bool ok = m_db.open();
    qDebug() << __FILE__ << "DB status:" << ok;
    qDebug() << m_db.lastError();



    //Check for tables and create them if missing
    if (!checkForTables()) exit(-1);
}

TrackCollection::~TrackCollection()
{
    // Save all tracks that haven't been saved yet.
    m_trackDao.saveDirtyTracks();

    Q_ASSERT(!m_db.rollback()); //Rollback any uncommitted transaction
    //The above is an ASSERT because there should never be an outstanding 
    //transaction when this code is called. If there is, it means we probably
    //aren't committing a transaction somewhere that should be.
    m_db.close();
    qDebug() << "TrackCollection destroyed";
}

bool TrackCollection::checkForTables()
{
    if (!m_db.open()) {
        QMessageBox::critical(0, qApp->tr("Cannot open database"),
                              qApp->tr("Unable to establish a database connection.\n"
                                       "Mixxx requires QT with SQLite support. Please read "
                                       "the Qt SQL driver documentation for information on how "
                                       "to build it.\n\n"
                                       "Click OK to exit."), QMessageBox::Ok);
        return false;
    }

    int requiredSchemaVersion = 4;
    if (!SchemaManager::upgradeToSchemaVersion(m_pConfig, m_db,
                                               requiredSchemaVersion)) {
        QMessageBox::warning(0, qApp->tr("Cannot upgrade database schema"),
                              qApp->tr(QString(
                                  "Unable to upgrade your database schema to version %1.\n"
                                  "Your mixxx.db file may be corrupt.\n"
                                  "Try renaming it and restarting Mixxx.\n\n"
                                  "Click OK to exit.").arg(requiredSchemaVersion)),
                             QMessageBox::Ok);
        return false;
    }

    m_trackDao.initialize();
    m_playlistDao.initialize();
    m_crateDao.initialize();
    m_cueDao.initialize();

    return true;
}


QSqlDatabase& TrackCollection::getDatabase()
{
 	return m_db;
  }


/** Do a non-recursive import of all the songs in a directory. Does NOT decend into subdirectories.
    @param trackDao The track data access object which provides a connection to the database. We use this parameter in order to make this function callable from separate threads. You need to use a different DB connection for each thread.
    @return true if the scan completed without being cancelled. False if the scan was cancelled part-way through.
*/
bool TrackCollection::importDirectory(QString directory, TrackDAO &trackDao)
{
    qDebug() << "TrackCollection::importDirectory(" << directory<< ")";

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
    QListIterator<QFileInfo> it(files);
    while (it.hasNext())
    {
        QFileInfo file = it.next(); //TODO: THIS IS SLOW!
        //If a flag was raised telling us to cancel the library scan then stop.
        m_libraryScanMutex.lock();
        bool cancel = bCancelLibraryScan;
        m_libraryScanMutex.unlock();
        if (cancel)
        {
            return false;
        }

        if (file.fileName().count(QRegExp(SoundSourceProxy::supportedFileExtensionsRegex(), Qt::CaseInsensitive))) {
            trackDao.markTrackLocationAsVerified(file.absoluteFilePath());

            //If the file already exists in the database, continue and go on to the next file.
            if (trackDao.trackExistsInDatabase(file.absoluteFilePath()))
            {
                continue;
                //Note that by checking if the track _exists_ in the DB, we also prevent Mixxx from
                //re-adding tracks that have been "removed" from the library. (That's tracks
                //where the mixxx_deleted column is 1. When you right-click and select
                //"Remove..." in the library, it sets that flag on the track in the DB rather
                //than actually deleting the row.)
            }
            //Load the song into a TrackInfoObject.
            emit(progressLoading(file.fileName()));
            //qDebug() << "Loading" << file.fileName();

            // TODO(XXX) addTrack repeats the work of using QFileInfo
            trackDao.addTrack(file.absoluteFilePath());

        } else {
            //qDebug() << "Skipping" << file.fileName() <<
            //    "because it did not match thesupported audio files filter:" <<
        }

    }
    emit(finishedLoading());
    return true;
}



void TrackCollection::slotCancelLibraryScan()
{
    m_libraryScanMutex.lock();
    bCancelLibraryScan = 1;
    m_libraryScanMutex.unlock();
}

void TrackCollection::resetLibaryCancellation()
{
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

