#include <QtCore>
#include <QtGui>
#include <QtSql>
#include <QtDebug>

#include "trackcollection.h"

#include "defs.h"
#include "library/librarytablemodel.h"
#include "library/schemamanager.h"
#include "library/queryutil.h"
#include "soundsourceproxy.h"
#include "trackinfoobject.h"
#include "xmlparse.h"
#include "util/sleepableqthread.h"

#include "controlobjectthread.h"
#include "controlobject.h"
#include "configobject.h"

#define MAX_LAMBDA_COUNT 8

TrackCollection::TrackCollection(ConfigObject<ConfigValue>* pConfig)
    : m_pConfig(pConfig),
      m_database(NULL),
      m_playlistDao(NULL),
      m_crateDao(NULL),
      m_cueDao(NULL),
      m_analysisDao(NULL),
      m_trackDao(NULL),
      m_stop(false),
      m_semLambdasReadyToCall(0),
      m_semLambdasFree(MAX_LAMBDA_COUNT),
      m_pCOTPlaylistIsBusy(NULL),
      m_supportedFileExtensionsRegex(
          SoundSourceProxy::supportedFileExtensionsRegex(),
          Qt::CaseInsensitive) {
    DBG() << "TrackCollection constructor \tfrom thread id="
          << QThread::currentThreadId() << "name="
          << QThread::currentThread()->objectName();
}

TrackCollection::~TrackCollection() {
    DBG() << "~TrackCollection()";
    m_trackDao->finish();

    if (m_database->isOpen()) {
        // There should never be an outstanding transaction when this code is
        // called. If there is, it means we probably aren't committing a
        // transaction somewhere that should be.
        if (m_database->rollback()) {
            qDebug() << "ERROR: There was a transaction in progress on the main database connection while shutting down."
                     << "There is a logic error somewhere.";
        }
        m_database->close();
    } else {
        qDebug() << "ERROR: The main database connection was closed before TrackCollection closed it."
                 << "There is a logic error somewhere.";
    }
    delete m_playlistDao;
    delete m_crateDao;
    delete m_cueDao;
    delete m_analysisDao;
    delete m_trackDao;
}

void TrackCollection::run() {
    QThread::currentThread()->setObjectName("TrackCollection");
    DBG() << "id=" << QThread::currentThreadId()
          << "name=" << QThread::currentThread()->objectName();

    DBG() << "Initializing DAOs inside TrackCollection's thread";

    createAndPopulateDbConnection();

    m_trackDao->initialize();
    m_playlistDao->initialize();
    m_crateDao->initialize();
    m_cueDao->initialize();

    m_pCOTPlaylistIsBusy = new ControlObjectThread(ConfigKey("[Playlist]", "isBusy"));
    Q_ASSERT(m_pCOTPlaylistIsBusy!=NULL);

    emit(initialized()); // to notify that Daos can be used

    func lambda;
    // main TrackCollection's loop
    while (!m_stop) {
        if (!m_semLambdasReadyToCall.tryAcquire(1)) {
            // no Lambda available, so unlock GUI.
            m_pCOTPlaylistIsBusy->set(0.0);
            m_semLambdasReadyToCall.acquire(1); // Sleep until new Lambdas have arrived
        }
        // got lambda on queue (or needness to stop thread)
        if (m_stop) {
            DBG() << "Need to stop thread";
            break;
        }
        if (!m_lambdas.isEmpty()) {
            m_lambdasQueueMutex.lock();
            lambda = m_lambdas.dequeue();
            m_lambdasQueueMutex.unlock();
            lambda();
            m_semLambdasFree.release(1);
        }
    }
    deleteLater();
    DBG() << " ### Thread ended ###";
}

// callAsync calls from GUI thread.
void TrackCollection::callAsync(func lambda, QString where) {
    qDebug() << "callAsync from" << where;
    if (lambda == NULL) return;
    Q_ASSERT(m_pCOTPlaylistIsBusy!=NULL);
    // lock GUI elements by setting [playlist] "isBusy"
    m_pCOTPlaylistIsBusy->set(1.0);
    addLambdaToQueue(lambda);
}

void TrackCollection::callSync(func lambda, QString where) {
    QMutex mutex;
    //SleepableQThread::sleep(5);
    mutex.lock();
    callAsync( [&mutex, &lambda] (void) {
        lambda();
        //SleepableQThread::sleep(5);
        mutex.unlock();
    }, where);

    while (!mutex.tryLock(5)) {
        // TODO(tro) this must not happen because we have possible race conditions
        // Maybe we must somehow allow only one callSync at a time
        MainExecuter::getInstance().call();
        // DBG() << "Start animation";
        // animationIsShowed = true;
    }
    mutex.unlock(); // QMutexes should be always destroyed in unlocked state.
}

void TrackCollection::addLambdaToQueue(func lambda) {
    //TODO(tro) check lambda
    m_semLambdasFree.acquire(1); // we'll wait here if lambdas count in queue is greater then MAX_LAMBDA_COUNT
    m_lambdasQueueMutex.lock();
    m_lambdas.enqueue(lambda);
    m_lambdasQueueMutex.unlock();
    m_semLambdasReadyToCall.release(1);
}

void TrackCollection::stopThread() {
    // TODO(tro) Think on how to do canceling
    DBG() << "Stopping thread";
    m_stop = true;
    m_semLambdasReadyToCall.release(1);
}


bool TrackCollection::checkForTables() {
    if (!m_database->open()) {
        QMessageBox::critical(0, tr("Cannot open database"),
                              tr("Unable to establish a database connection.\n"
                                 "Mixxx requires Qt with SQLite support. Please read "
                                 "the Qt SQL driver documentation for information on how "
                                 "to build it.\n\n"
                                 "Click OK to exit."), QMessageBox::Ok);
        return false;
    }

    int requiredSchemaVersion = 20; // TODO avoid constant 20
    QString schemaFilename = m_pConfig->getResourcePath();
    schemaFilename.append("schema.xml");
    QString okToExit = tr("Click OK to exit.");
    QString upgradeFailed = tr("Cannot upgrade database schema");
    QString upgradeToVersionFailed = tr("Unable to upgrade your database schema to version %1")
            .arg(QString::number(requiredSchemaVersion));
    int result = SchemaManager::upgradeToSchemaVersion(schemaFilename, *m_database, requiredSchemaVersion);
    if (result < 0) {
        if (result == -1) {
            QMessageBox::warning(0, upgradeFailed,
                                 upgradeToVersionFailed + "\n" +
                                 tr("Your %1 file may be outdated.").arg(schemaFilename) +
                                 "\n\n" + okToExit,
                                 QMessageBox::Ok);
        } else if (result == -2) {
            QMessageBox::warning(0, upgradeFailed,
                                 upgradeToVersionFailed + "\n" +
                                 tr("Your mixxxdb.sqlite file may be corrupt.") + "\n" +
                                 tr("Try renaming it and restarting Mixxx.") +
                                 "\n\n" + okToExit,
                                 QMessageBox::Ok);
        } else { // -3
            QMessageBox::warning(0, upgradeFailed,
                                 upgradeToVersionFailed + "\n" +
                                 tr("Your %1 file may be missing or invalid.").arg(schemaFilename) +
                                 "\n\n" + okToExit,
                                 QMessageBox::Ok);
        }
        return false;
    }
    return true;
}

QSqlDatabase& TrackCollection::getDatabase() {
    return *m_database;
}

/** Do a non-recursive import of all the songs in a directory. Does NOT decend into subdirectories.
    @param trackDao The track data access object which provides a connection to the database. We use this parameter in order to make this function callable from separate threads. You need to use a different DB connection for each thread.
    @return true if the scan completed without being cancelled. False if the scan was cancelled part-way through.
*/
bool TrackCollection::importDirectory(const QString& directory, TrackDAO& trackDao,
                                      const QStringList& nameFilters,
                                      volatile bool* cancel) {
    //qDebug() << "TrackCollection::importDirectory(" << directory<< ")";

    emit(startedLoading());
    // QFileInfoList files;

    //get a list of the contents of the directory and go through it.
    QDirIterator it(directory, nameFilters, QDir::Files | QDir::NoDotAndDotDot);
    while (it.hasNext()) {

        //If a flag was raised telling us to cancel the library scan then stop.
        if (*cancel) {
            return false;
        }

        QString absoluteFilePath = it.next();

        // If the track is in the database, mark it as existing. This code gets exectuted
        // when other files in the same directory have changed (the directory hash has changed).
        trackDao.markTrackLocationAsVerified(absoluteFilePath);

        // If the file already exists in the database, continue and go on to
        // the next file.

        // If the file doesn't already exist in the database, then add
        // it. If it does exist in the database, then it is either in the
        // user's library OR the user has "removed" the track via
        // "Right-Click -> Remove". These tracks stay in the library, but
        // their mixxx_deleted column is 1.
        if (!trackDao.trackExistsInDatabase(absoluteFilePath)) {
            //qDebug() << "Loading" << it.fileName();
            emit(progressLoading(it.fileName()));

            TrackPointer pTrack = TrackPointer(new TrackInfoObject(
                                                   absoluteFilePath), &QObject::deleteLater);

            if (trackDao.addTracksAdd(pTrack.data(), false)) {
                // Successful added
                // signal the main instance of TrackDao, that there is a
                // new Track in the database
                m_trackDao->databaseTrackAdded(pTrack);
            } else {
                qDebug() << "Track ("+absoluteFilePath+") could not be added";
            }
        }
    }
    emit(finishedLoading());
    return true;
}

CrateDAO& TrackCollection::getCrateDAO() {
    return *m_crateDao;
}

TrackDAO& TrackCollection::getTrackDAO() {
    return *m_trackDao;
}

PlaylistDAO& TrackCollection::getPlaylistDAO() {
    return *m_playlistDao;
}

QSharedPointer<BaseTrackCache> TrackCollection::getTrackSource(
        const QString& name) {
    return m_trackSources.value(name, QSharedPointer<BaseTrackCache>());
}

void TrackCollection::addTrackSource(
        const QString& name, QSharedPointer<BaseTrackCache> trackSource) {
    Q_ASSERT(!m_trackSources.contains(name));
    m_trackSources[name] = trackSource;
}

void TrackCollection::createAndPopulateDbConnection() {

    // initialize database connection in ThreadDAO's thread -- copypaste from TrackCollection
    qDebug() << "Available QtSQL drivers:" << QSqlDatabase::drivers();
    // TODO(tro) Check is QSQLITE is avaiable
    m_database = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"));

    m_database->setHostName("localhost");
    m_database->setDatabaseName(m_pConfig->getSettingsPath().append("/mixxxdb.sqlite"));
    m_database->setUserName("mixxx");
    m_database->setPassword("mixxx");
    bool ok = m_database->open();
    qDebug() << "DB status:" << m_database->databaseName() << "=" << ok;
    if (m_database->lastError().isValid()) {
        qDebug() << "Error loading database:" << m_database->lastError();
    }
    // Check for tables and create them if missing
    if (!checkForTables()) {
        // TODO(XXX) something a little more elegant
        exit(-1);
    }

    m_playlistDao = new PlaylistDAO(*m_database);
    m_crateDao = new CrateDAO(*m_database);
    m_cueDao = new CueDAO(*m_database);
    m_analysisDao = new AnalysisDao(*m_database, m_pConfig);
    m_trackDao = new TrackDAO(*m_database, *m_cueDao, *m_playlistDao, *m_crateDao, *m_analysisDao, m_pConfig);
}
