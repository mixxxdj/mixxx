#include "repository/repository.h"

#include "repository/schemamanager.h"

#include "util/assert.h"
#include "util/logger.h"


namespace mixxx {

//static
const QString Repository::kDefaultDatabaseConnectionName = "MIXXX";

// The schema XML is baked into the binary via Qt resources.
//static
const QString Repository::kDefaultSchemaFile(":/schema.xml");

//static
const int Repository::kRequiredSchemaVersion = 27;

namespace {

const Logger kLogger("Repository");

const QString kDatabaseFileName = "mixxxdb.sqlite";

} // anonymous namespace

Repository::Repository(
        const UserSettingsPointer& pConfig,
        const QString& dbConnectionName)
    : m_dbConnection(pConfig->getSettingsPath(), kDatabaseFileName, dbConnectionName) {
}

bool Repository::initDatabaseSchema(
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

} // namespace mixxx
