#include <QDebug>
#include <QEventLoop>
#include <QApplication>
#include <QPushButton>
#include <QMessageBox>
#include <QDir>

#include "threaddao.h"
#include "util/sleepableqthread.h"
#include "library/schemamanager.h"

const int SleepTimeMs = 10;
const int TooLong = 100;

ThreadDAO::ThreadDAO(ConfigObject<ConfigValue> *pConfig, QObject *parent)
    : QThread(parent),
      m_pConfig(pConfig),
      m_database(QSqlDatabase::addDatabase("QSQLITE")),
      m_playlistDao(m_database),
      m_crateDao(m_database),
      m_cueDao(m_database),
      m_analysisDao(m_database, pConfig),
      m_trackDao(m_database, m_cueDao, m_playlistDao, m_crateDao,
                 m_analysisDao, pConfig),
      m_haveFunction(false),
      m_callFinished(false),
      m_stop(false),
      m_databasePath(pConfig->getSettingsPath().append("/mixxxdb.sqlite")) {
    qDebug() << "### ThreadDAO constructor ###";
    qDebug() << "\tfrom thread id=" << QThread::currentThreadId()
             << "name=" << QThread::currentThread()->objectName();
}

ThreadDAO::~ThreadDAO() {
    qDebug() << "### ThreadDAO destructor ###";

    if (isRunning()) {
        // TODO(tro) Cancel any running queries
        stopThread();
        wait(); // Wait for thread to finish
    }

    m_trackDao.finish();

    if (m_database.isOpen()) {
        // There should never be an outstanding transaction when this code is
        // called. If there is, it means we probably aren't committing a
        // transaction somewhere that should be.
        if (m_database.rollback()) {
            qDebug() << "ERROR: There was a transaction in progress on the main database connection while shutting down."
                     << "There is a logic error somewhere.";
        }
        m_database.close();
    } else {
        qDebug() << "ERROR: The main database connection was closed before TrackCollection closed it."
                 << "There is a logic error somewhere.";
    }

}


void ThreadDAO::run() {
    QThread::currentThread()->setObjectName("ThreadDAO");
    qDebug() << "ThreadDAO::run, id=" << QThread::currentThreadId()
             << "name=" << QThread::currentThread()->objectName();

    // initialize database connection in ThreadDAO's thread -- copypaste from TrackCollection
    qDebug() << "Available QtSQL drivers:" << QSqlDatabase::drivers();

    m_database.setHostName("localhost");
    m_database.setDatabaseName(m_databasePath);
    m_database.setUserName("mixxx");
    m_database.setPassword("mixxx");
    bool ok = m_database.open();
    qDebug() << "DB status:" << m_database.databaseName() << "=" << ok;
    if (m_database.lastError().isValid()) {
        qDebug() << "Error loading database:" << m_database.lastError();
    }
    // Check for tables and create them if missing
    if (!checkForTables()) {
        // TODO(XXX) something a little more elegant
        exit(-1);
    }

    qDebug() << "### Initializing DAOs inside ThreadDAO's thread";
    m_trackDao.initialize();
    m_playlistDao.initialize();
    m_crateDao.initialize();
    m_cueDao.initialize();


    // main ThreadDAO's loop
    while (!m_stop) {
        while (!m_haveFunction) {
            SleepableQThread::msleep(SleepTimeMs);
        }
        m_callFinished = false;

        m_lambda();

        m_haveFunction = false;
        m_callFinished = true;
    }
}

// tro: callSync must be executed in GUI thread. callSync waits while and
//      process application events while lambda is executing.
//      input: lambda - function that must be executed in ThredDAO's thread
//             second parameter - UI's control, what must be locked/unlocked
//             while lambda will be executed.
void ThreadDAO::callSync(func lambda, QWidget &w) {
    //TODO(tro) check lambda

    int waitCycles = 0;
    while(!m_callFinished) {
        qApp->processEvents( QEventLoop::AllEvents );
        SleepableQThread::msleep(SleepTimeMs);
        ++waitCycles;
    }
    qDebug() << "\tWaited before setLambda:" << waitCycles;
    setLambda(lambda);
    waitCycles = 0;
    bool animationIsShowed = false;

    while (m_haveFunction && !m_callFinished) {
        qApp->processEvents( QEventLoop::AllEvents );
        SleepableQThread::msleep(SleepTimeMs);
        ++waitCycles;
        if (waitCycles > TooLong && !animationIsShowed) {
            qDebug() << "Start animation";
            w.setEnabled(false);
//            m_progressindicator.startAnimation();
            animationIsShowed = true;
        }
    }
    qDebug() << "\tWaited execution of lambda:" << waitCycles;

    if (animationIsShowed) {
        qDebug() << "Stop animation";
        w.setEnabled(true);
//        m_progressindicator.stopAnimation();
    }
}

void ThreadDAO::stopThread() {
    // TODO(tro) Think on how to do canceling
    qDebug() << "Stopping thread";
    m_stop = true;
}

// store lambda to private member
void ThreadDAO::setLambda(func lambda) {
    m_lambda = lambda;
    m_haveFunction = true;
    m_callFinished = false;
}

bool ThreadDAO::checkForTables() {
    if (!m_database.open()) {
        QMessageBox::critical(0, tr("Cannot open database"),
                            tr("Unable to establish a database connection.\n"
                                "Mixxx requires QT with SQLite support. Please read "
                                "the Qt SQL driver documentation for information on how "
                                "to build it.\n\n"
                                "Click OK to exit."), QMessageBox::Ok);
        return false;
    }

    int requiredSchemaVersion = 20;
    QString schemaFilename = m_pConfig->getResourcePath();
    schemaFilename.append("schema.xml");
    QString okToExit = tr("Click OK to exit.");
    QString upgradeFailed = tr("Cannot upgrade database schema");
    QString upgradeToVersionFailed = tr("Unable to upgrade your database schema to version %1")
            .arg(QString::number(requiredSchemaVersion));
    int result = SchemaManager::upgradeToSchemaVersion(schemaFilename, m_database, requiredSchemaVersion);
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

bool ThreadDAO::importDirectory(const QString& directory, TrackDAO& trackDao, const QStringList& nameFilters, volatile bool* cancel) {
    qDebug() << "ThreadDAO::importDirectory(" << directory<< ")";

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
//            emit(progressLoading(it.fileName()));

            TrackPointer pTrack = TrackPointer(new TrackInfoObject(
                              absoluteFilePath), &QObject::deleteLater);

            if (trackDao.addTracksAdd(pTrack.data(), false)) {
                // Successful added
                // signal the main instance of TrackDao, that there is a
                // new Track in the database
                m_trackDao.databaseTrackAdded(pTrack);
            } else {
                qDebug() << "Track ("+absoluteFilePath+") could not be added";
            }
        }
    }
//    emit(finishedLoading());
    return true;
}
