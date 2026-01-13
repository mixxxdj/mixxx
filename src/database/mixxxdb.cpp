#include "database/mixxxdb.h"

#include <QDir>
#include <QSqlQuery>
#include <QVariant>

#include "database/schemamanager.h"
#include "moc_mixxxdb.cpp"
#include "util/assert.h"
#include "util/logger.h"

// The schema XML is baked into the binary via Qt resources.
//static
const QString MixxxDb::kDefaultSchemaFile(":/schema.xml");

//static
const int MixxxDb::kRequiredSchemaVersion = 40;

namespace {

const mixxx::Logger kLogger("MixxxDb");

const QString kType = QStringLiteral("QSQLITE");

const QString kConnectOptions = QStringLiteral("QSQLITE_OPEN_URI");

const QString kUriPrefix = QStringLiteral("file://");

const QString kDefaultFileName = QStringLiteral("mixxxdb.sqlite");

const QString kUserName = QStringLiteral("mixxx");

const QString kPassword = QStringLiteral("mixxx");

bool ensureColumnExists(const QSqlDatabase& database,
        const QString& tableName,
        const QString& columnName,
        const QString& alterSql) {
    QSqlQuery pragmaQuery(database);
    pragmaQuery.prepare(QStringLiteral("PRAGMA table_info(%1)").arg(tableName));
    if (!pragmaQuery.exec()) {
        kLogger.warning() << "Failed to inspect table" << tableName
                          << pragmaQuery.lastError().text();
        return false;
    }

    while (pragmaQuery.next()) {
        if (pragmaQuery.value(QStringLiteral("name")).toString() == columnName) {
            return true;
        }
    }

    QSqlQuery alterQuery(database);
    if (!alterQuery.exec(alterSql)) {
        kLogger.warning() << "Failed to add column" << columnName << "to" << tableName
                          << alterQuery.lastError().text();
        return false;
    }

    kLogger.info() << "Added missing column" << columnName << "to table" << tableName;
    return true;
}

bool ensureTuningFrequencyColumn(const QSqlDatabase& database) {
    // Some users might have databases stuck before schema 40. Ensure the column exists
    // so queries using tuning_frequency_hz don't fail.
    return ensureColumnExists(
            database,
            QStringLiteral("library"),
            QStringLiteral("tuning_frequency_hz"),
            QStringLiteral("ALTER TABLE library ADD COLUMN tuning_frequency_hz INTEGER DEFAULT 440"));
}

bool ensureLibraryNotEmpty(const QSqlDatabase& database) {
    QSqlQuery countLibrary(database);
    if (!countLibrary.exec(QStringLiteral("SELECT COUNT(*) FROM library"))) {
        kLogger.warning() << "Failed to count library rows" << countLibrary.lastError().text();
        return false;
    }
    int libraryRows = 0;
    if (countLibrary.next()) {
        libraryRows = countLibrary.value(0).toInt();
    }

    QSqlQuery countTrackLocations(database);
    if (!countTrackLocations.exec(QStringLiteral("SELECT COUNT(*) FROM track_locations"))) {
        kLogger.warning() << "Failed to count track_locations rows"
                          << countTrackLocations.lastError().text();
        return false;
    }
    int trackLocationRows = 0;
    if (countTrackLocations.next()) {
        trackLocationRows = countTrackLocations.value(0).toInt();
    }

    // If the library table is empty but track_locations has rows, force a full rescan by
    // removing directory hashes and clearing track_locations. This recovers from
    // partially migrated databases that lost library rows but kept track locations,
    // which prevents re-import because the scanner thinks tracks already exist.
    if (libraryRows == 0 && trackLocationRows > 0) {
        kLogger.warning() << "Library table is empty but" << trackLocationRows
                          << "track locations exist; forcing full rescan";
        QSqlQuery deleteHashes(database);
        if (!deleteHashes.exec(QStringLiteral("DELETE FROM LibraryHashes"))) {
            kLogger.warning() << "Failed to clear LibraryHashes for recovery"
                              << deleteHashes.lastError().text();
            return false;
        }
        QSqlQuery deleteTrackLocations(database);
        if (!deleteTrackLocations.exec(QStringLiteral("DELETE FROM track_locations"))) {
            kLogger.warning() << "Failed to clear track_locations for recovery"
                              << deleteTrackLocations.lastError().text();
            return false;
        }
        kLogger.info() << "Cleared LibraryHashes and track_locations to allow full rescan";
    }
    return true;
}

// The connection parameters for the main Mixxx DB
mixxx::DbConnection::Params dbConnectionParams(
        const UserSettingsPointer& pConfig,
        bool inMemoryConnection) {
    mixxx::DbConnection::Params params;
    params.type = kType;
    params.connectOptions = kConnectOptions;
    params.filePath = kUriPrefix;
    const QString absFilePath =
            QDir(pConfig->getSettingsPath()).absoluteFilePath(kDefaultFileName);
    // On Windows absFilePath starts with a drive letter instead of
    // the leading '/' as required.
    // https://www.sqlite.org/c3ref/open.html#urifilenameexamples
    if (!absFilePath.startsWith(QChar('/'))) {
        params.filePath += QChar('/');
    }
    params.filePath += absFilePath;
    // Allow multiple connections to the same in-memory database by
    // using a named connection. This is needed to make the database
    // connection pool work correctly even during tests.
    //
    // See also:
    // https://www.sqlite.org/inmemorydb.html
    if (inMemoryConnection) {
        params.filePath += QStringLiteral("?mode=memory&cache=shared");
    }
    params.userName = kUserName;
    params.password = kPassword;
    return params;
}

} // anonymous namespace

MixxxDb::MixxxDb(
        const UserSettingsPointer& pConfig,
        bool inMemoryConnection)
    : m_pDbConnectionPool(std::make_shared<mixxx::DbConnectionPool>(dbConnectionParams(pConfig, inMemoryConnection), "MIXXX")) {
}

bool MixxxDb::initDatabaseSchema(
        const QSqlDatabase& database,
        int schemaVersion,
        const QString& schemaFile) {
    QString okToExit = tr("Click OK to exit.");
    QString upgradeFailed = tr("Cannot upgrade database schema");
    QString upgradeToVersionFailed =
            tr("Unable to upgrade your database schema to version %1")
            .arg(QString::number(schemaVersion));
    QString helpContact = tr("For help with database issues consult:") + "\n" +
            "https://www.mixxx.org/support";

    switch (SchemaManager(database).upgradeToSchemaVersion(schemaVersion, schemaFile)) {
    case SchemaManager::Result::CurrentVersion:
    case SchemaManager::Result::UpgradeSucceeded:
    case SchemaManager::Result::NewerVersionBackwardsCompatible:
        // Ensure tuning_frequency_hz exists even if a previous upgrade was skipped.
        if (!ensureTuningFrequencyColumn(database) ||
                !ensureLibraryNotEmpty(database)) {
            QMessageBox::warning(nullptr,
                    upgradeFailed,
                    upgradeToVersionFailed + "\n" +
                            tr("Could not ensure database schema consistency.") +
                            "\n" + helpContact + "\n\n" + okToExit,
                    QMessageBox::Ok);
            return false;
        }
        return true; // done
    case SchemaManager::Result::UpgradeFailed:
        QMessageBox::warning(nullptr,
                upgradeFailed,
                upgradeToVersionFailed + "\n" +
                        tr("Your mixxxdb.sqlite file may be corrupt.") +
                        "\n" + tr("Try renaming it and restarting Mixxx.") +
                        "\n" + helpContact + "\n\n" + okToExit,
                QMessageBox::Ok);
        return false; // abort
    case SchemaManager::Result::NewerVersionIncompatible:
        QMessageBox::warning(nullptr,
                upgradeFailed,
                upgradeToVersionFailed + "\n" +
                        tr("Your mixxxdb.sqlite file was created by a newer "
                           "version of Mixxx and is incompatible.") +
                        "\n\n" + okToExit,
                QMessageBox::Ok);
        return false; // abort
    case SchemaManager::Result::SchemaError:
        QMessageBox::warning(nullptr,
                upgradeFailed,
                upgradeToVersionFailed + "\n" +
                        tr("The database schema file is invalid.") + "\n" +
                        helpContact + "\n\n" + okToExit,
                QMessageBox::Ok);
        return false; // abort
    }
    // Suppress compiler warning
    DEBUG_ASSERT(!"unhandled switch/case");
    return false;
}
