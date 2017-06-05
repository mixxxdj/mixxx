#include "database/mixxxdb.h"

#include "database/schemamanager.h"

#include "util/assert.h"
#include "util/logger.h"


// The schema XML is baked into the binary via Qt resources.
//static
const QString MixxxDB::kDefaultSchemaFile(":/schema.xml");

// static
const int MixxxDB::kRequiredSchemaVersion = 27;

namespace {
    mixxx::Logger kLogger("MixxxDB");
} // anonymous namespace

MixxxDB::MixxxDB(
        UserSettingsPointer pConfig)
    : m_dbConnection(pConfig->getSettingsPath()) {
}

bool MixxxDB::initDatabaseSchema(
        const QString& schemaFile,
        int schemaVersion) {
    if (!database().isOpen()) {
        QMessageBox::critical(0, tr("Cannot open database"),
                            tr("Unable to establish a database connection.\n"
                                "Mixxx requires QT with SQLite support. Please read "
                                "the Qt SQL driver documentation for information on how "
                                "to build it.\n\n"
                                "Click OK to exit."), QMessageBox::Ok);
        return false; // abort
    }

    QString okToExit = tr("Click OK to exit.");
    QString upgradeFailed = tr("Cannot upgrade database schema");
    QString upgradeToVersionFailed =
            tr("Unable to upgrade your database schema to version %1")
            .arg(QString::number(schemaVersion));
    QString helpEmail = tr("For help with database issues contact:") + "\n" +
                           "mixxx-devel@lists.sourceforge.net";

    SchemaManager schemaManager(database());
    switch (schemaManager.upgradeToSchemaVersion(schemaFile, schemaVersion)) {
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
