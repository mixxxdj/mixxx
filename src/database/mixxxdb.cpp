#include "database/mixxxdb.h"

#include "database/schemamanager.h"

#include "util/assert.h"
#include "util/logger.h"


// The schema XML is baked into the binary via Qt resources.
//static
const QString MixxxDb::kDefaultSchemaFile(":/schema.xml");

//static
const int MixxxDb::kRequiredSchemaVersion = 29;

namespace {

const mixxx::Logger kLogger("MixxxDb");

// The connection parameters for the main Mixxx DB
mixxx::DbConnection::Params dbConnectionParams(
        const UserSettingsPointer& pConfig,
        bool inMemoryConnection) {
    mixxx::DbConnection::Params params;
    params.type = "QSQLITE";
    params.hostName = "localhost";
    params.filePath = inMemoryConnection ? QString(":memory:") : QDir(pConfig->getSettingsPath()).filePath("mixxxdb.sqlite");
    params.userName = "mixxx";
    params.password = "mixxx";
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
        const QString& schemaFile,
        int schemaVersion) {
    QString okToExit = tr("Click OK to exit.");
    QString upgradeFailed = tr("Cannot upgrade database schema");
    QString upgradeToVersionFailed =
            tr("Unable to upgrade your database schema to version %1")
            .arg(QString::number(schemaVersion));
    QString helpEmail = tr("For help with database issues contact:") + "\n" +
                           "mixxx-devel@lists.sourceforge.net";

    switch (SchemaManager(database).upgradeToSchemaVersion(schemaFile, schemaVersion)) {
        case SchemaManager::Result::CurrentVersion:
        case SchemaManager::Result::UpgradeSucceeded:
        case SchemaManager::Result::NewerVersionBackwardsCompatible:
            return true; // done
        case SchemaManager::Result::UpgradeFailed:
            QMessageBox::warning(
                    0, upgradeFailed,
                    upgradeToVersionFailed + "\n" +
                    tr("Your mixxxdb.sqlite file may be corrupt.") + "\n" +
                    tr("Try renaming it and restarting Mixxx.") + "\n" +
                    helpEmail + "\n\n" + okToExit,
                    QMessageBox::Ok);
            return false; // abort
        case SchemaManager::Result::NewerVersionIncompatible:
            QMessageBox::warning(
                    0, upgradeFailed,
                    upgradeToVersionFailed + "\n" +
                    tr("Your mixxxdb.sqlite file was created by a newer "
                       "version of Mixxx and is incompatible.") +
                    "\n\n" + okToExit,
                    QMessageBox::Ok);
            return false; // abort
        case SchemaManager::Result::SchemaError:
            QMessageBox::warning(
                    0, upgradeFailed,
                    upgradeToVersionFailed + "\n" +
                    tr("The database schema file is invalid.") + "\n" +
                    helpEmail + "\n\n" + okToExit,
                    QMessageBox::Ok);
            return false; // abort
    }
    // Suppress compiler warning
    DEBUG_ASSERT(!"unhandled switch/case");
    return false;
}
