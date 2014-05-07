#include <QtSql>
#ifdef __SQLITE3__
#include <sqlite3.h>
#endif
#include <QtDebug>

#include "library/trackcollection.h"
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
          m_directoryDao(m_db),
          m_analysisDao(m_db, pConfig),
          m_trackDao(m_db, m_cueDao, m_playlistDao, m_crateDao,
                     m_analysisDao, m_directoryDao, pConfig),
          m_supportedFileExtensionsRegex(
              SoundSourceProxy::supportedFileExtensionsRegex(),
              Qt::CaseInsensitive) {
    qDebug() << "Available QtSQL drivers:" << QSqlDatabase::drivers();

    m_db.setHostName("localhost");
    m_db.setDatabaseName(pConfig->getSettingsPath().append("/mixxxdb.sqlite"));
    m_db.setUserName("mixxx");
    m_db.setPassword("mixxx");
    bool ok = m_db.open();
    qDebug() << "DB status:" << m_db.databaseName() << "=" << ok;
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

    int requiredSchemaVersion = 23;
    QString schemaFilename = m_pConfig->getResourcePath();
    schemaFilename.append("schema.xml");
    QString okToExit = tr("Click OK to exit.");
    QString upgradeFailed = tr("Cannot upgrade database schema");
    QString upgradeToVersionFailed = tr("Unable to upgrade your database schema to version %1")
            .arg(QString::number(requiredSchemaVersion));
    int result = SchemaManager::upgradeToSchemaVersion(schemaFilename, m_db, requiredSchemaVersion);
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

#ifdef __SQLITE3__
    installSorting(m_db);
#endif

    m_trackDao.initialize();
    m_playlistDao.initialize();
    m_crateDao.initialize();
    m_cueDao.initialize();
    m_directoryDao.initialize();

    return true;
}

QSqlDatabase& TrackCollection::getDatabase() {
    return m_db;
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

DirectoryDAO& TrackCollection::getDirectoryDAO() {
    return m_directoryDao;
}

QSharedPointer<BaseTrackCache> TrackCollection::getTrackSource() {
    return m_defaultTrackSource;
}

void TrackCollection::setTrackSource(QSharedPointer<BaseTrackCache> trackSource) {
    Q_ASSERT(m_defaultTrackSource.isNull());
    m_defaultTrackSource = trackSource;
}

#ifdef __SQLITE3__
// from public domain code
// http://www.archivum.info/qt-interest@trolltech.com/2008-12/00584/Re-%28Qt-interest%29-Qt-Sqlite-UserDefinedFunction.html
void TrackCollection::installSorting( QSqlDatabase &db) {
    QVariant v = db.driver()->handle();
    if (v.isValid() && strcmp(v.typeName(), "sqlite3*") == 0) {
        // v.data() returns a pointer to the handle
        sqlite3* handle = *static_cast<sqlite3**>(v.data());
        if (handle != 0) { // check that it is not NULL
            int result = sqlite3_create_collation(
                    handle,
                    "localeAwareCompare",
                    SQLITE_UTF16, // ANY would be nice, but we only  encode in 16 anyway.
                    0,
                    sqliteLocaleAwareCompare);
            if (result != SQLITE_OK)
            qWarning() << "Could not add string collation function: " << result;
        } else {
            qWarning() << "Could not get sqlite handle";
        }
    } else {
        qWarning() << "handle variant returned typename " << v.typeName();
    }
}

// The collating function callback is invoked with a copy of the pArg
// application data pointer and with two strings in the encoding specified
// by the eTextRep argument.
// The collating function must return an integer that is negative, zero,
// or positive if the first string is less than, equal to, or greater
// than the second, respectively.
//static
int TrackCollection::sqliteLocaleAwareCompare(void* pArg,
                                    int len1, const void* data1,
                                    int len2, const void* data2 )
{
    Q_UNUSED(pArg);
    // Construct a QString without copy
    QString string1 = QString::fromRawData(reinterpret_cast<const QChar*>(data1),
                                           len1 / sizeof(QChar));
    QString string2 = QString::fromRawData(reinterpret_cast<const QChar*>(data2),
                                           len2 / sizeof(QChar));
    return QString::localeAwareCompare(string1, string2);
}
#endif
