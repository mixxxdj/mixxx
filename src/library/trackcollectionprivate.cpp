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
#ifdef __AUTODJCRATES__
#include "library/dao/autodjcratesdao.h"
#endif


#define MAX_LAMBDA_COUNT 8

TrackCollectionPrivate::TrackCollectionPrivate(ConfigObject<ConfigValue>* pConfig)
        : m_pConfig(pConfig),
          m_pDatabase(NULL),
          m_pPlaylistDao(NULL),
          m_pCrateDao(NULL),
          m_pCueDao(NULL),
          m_pAnalysisDao(NULL),
          m_pTrackDao(NULL),
		  m_pAutoDjCratesDao(NULL),
          m_supportedFileExtensionsRegex(
                  SoundSourceProxy::supportedFileExtensionsRegex(),
                  Qt::CaseInsensitive){
    DBG() << "TrackCollectionPrivate constructor \tfrom thread id="
          << QThread::currentThreadId() << "name="
          << QThread::currentThread()->objectName();
}

TrackCollectionPrivate::~TrackCollectionPrivate() {
    DBG() << "~TrackCollectionPrivate()";
    delete m_pCOTPlaylistIsBusy;
}

void TrackCollectionPrivate::initialize(){
    qRegisterMetaType<QSet<int> >("QSet<int>");

    createAndPopulateDbConnection();

    DBG() << "Initializing DAOs inside TrackCollectionPrivate";
    m_pTrackDao->initialize();
    m_pPlaylistDao->initialize();
    m_pCrateDao->initialize();
    m_pCueDao->initialize();
}

bool TrackCollectionPrivate::checkForTables() {
    if (!m_pDatabase->open()) {
        MainExecuter::callSync([this](void) {
            QMessageBox::critical(0, tr("Cannot open database"),
                                  tr("Unable to establish a database connection.\n"
                                     "Mixxx requires Qt with SQLite support. Please read "
                                     "the Qt SQL driver documentation for information on how "
                                     "to build it.\n\n"
                                     "Click OK to exit."), QMessageBox::Ok);
        });
        return false;
    }

    bool checkResult = true;
    MainExecuter::callSync([this, &checkResult](void) {
        int requiredSchemaVersion = 20; // TODO(xxx) avoid constant 20
        QString schemaFilename = m_pConfig->getResourcePath();
        schemaFilename.append("schema.xml");
        QString okToExit = tr("Click OK to exit.");
        QString upgradeFailed = tr("Cannot upgrade database schema");
        QString upgradeToVersionFailed = tr("Unable to upgrade your database schema to version %1")
                .arg(QString::number(requiredSchemaVersion));
        int result = SchemaManager::upgradeToSchemaVersion(schemaFilename, *m_pDatabase, requiredSchemaVersion);
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
        }
        checkResult = false;
    });
    return checkResult;
}

QSqlDatabase& TrackCollectionPrivate::getDatabase() {
    return *m_pDatabase;
}

CrateDAO& TrackCollectionPrivate::getCrateDAO() {
    return *m_pCrateDao;
}

TrackDAO& TrackCollectionPrivate::getTrackDAO() {
    return *m_pTrackDao;
}

PlaylistDAO& TrackCollectionPrivate::getPlaylistDAO() {
    return *m_pPlaylistDao;
}

#ifdef __AUTODJCRATES__
AutoDJCratesDAO& TrackCollectionPrivate::getAutoDJCratesDAO(){
	return *m_pAutoDjCratesDao;
}
#endif

void TrackCollectionPrivate::createAndPopulateDbConnection() {
    // initialize database connection in TrackCollection
    const QStringList avaiableDrivers = QSqlDatabase::drivers();
    const QString sqliteDriverName("QSQLITE");

    if (!avaiableDrivers.contains(sqliteDriverName)) {
        QString errorMsg = QString("No QSQLITE driver! Available QtSQL drivers: %1")
                .arg(avaiableDrivers.join(","));
        qDebug() << errorMsg;
        exit(-1);
    }

    m_pDatabase = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", "track_collection_connection"));
    m_pDatabase->setHostName("localhost");
    m_pDatabase->setDatabaseName(m_pConfig->getSettingsPath().append("/mixxxdb.sqlite"));
    m_pDatabase->setUserName("mixxx");
    m_pDatabase->setPassword("mixxx");
    bool ok = m_pDatabase->open();
    qDebug() << "DB status:" << m_pDatabase->databaseName() << "=" << ok;
    if (m_pDatabase->lastError().isValid()) {
        qDebug() << "Error loading database:" << m_pDatabase->lastError();
    }
    // Check for tables and create them if missing
    if (!checkForTables()) {
        // TODO(XXX) something a little more elegant
        exit(-1);
    }

    m_pPlaylistDao = new PlaylistDAO(*m_pDatabase);
    m_pCrateDao = new CrateDAO(*m_pDatabase);
    m_pCueDao = new CueDAO(*m_pDatabase);
    m_pAnalysisDao = new AnalysisDao(*m_pDatabase, m_pConfig);
    m_pTrackDao = new TrackDAO(*m_pDatabase, *m_pCueDao, *m_pPlaylistDao, *m_pCrateDao, *m_pAnalysisDao, m_pConfig);
#ifdef __AUTODJCRATES__
		m_pAutoDjCratesDao = new AutoDJCratesDAO(*m_pDatabase,
												 *m_pTrackDao,
												 *m_pCrateDao,
												 *m_pPlaylistDao,
												 m_pConfig);
#endif // __AUTODJCRATES__
}
