#include "database/mixxxdb.h"

#include "database/schemamanager.h"
#include "moc_mixxxdb.cpp"
#include "util/assert.h"
#include "util/logger.h"

// The schema XML is baked into the binary via Qt resources.
//static
const QString MixxxDb::kDefaultSchemaFile(":/schema.xml");

//static
const int MixxxDb::kRequiredSchemaVersion = 35;

namespace {

const mixxx::Logger kLogger("MixxxDb");

const QString kType = QStringLiteral("QSQLITE");

const QString kConnectOptions = QStringLiteral("QSQLITE_OPEN_URI");

const QString kUriPrefix = QStringLiteral("file://");

const QString kDefaultFileName = QStringLiteral("mixxxdb.sqlite");

const QString kUserName = QStringLiteral("mixxx");

const QString kPassword = QStringLiteral("mixxx");

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
