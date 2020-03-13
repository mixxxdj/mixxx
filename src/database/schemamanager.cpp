#include "database/schemamanager.h"

#include "util/db/fwdsqlquery.h"
#include "util/db/sqltransaction.h"
#include "util/xml.h"
#include "util/logger.h"
#include "util/assert.h"


const QString SchemaManager::SETTINGS_VERSION_STRING = "mixxx.schema.version";
const QString SchemaManager::SETTINGS_MINCOMPATIBLE_STRING = "mixxx.schema.min_compatible_version";

namespace {
    mixxx::Logger kLogger("SchemaManager");

    int readCurrentSchemaVersion(SettingsDAO& settings) {
        QString settingsValue = settings.getValue(SchemaManager::SETTINGS_VERSION_STRING);
        // May be a null string if the schema has not been created. We default the
        // startVersion to 0 so that we automatically try to upgrade to revision 1.
        if (settingsValue.isNull()) {
            return 0; // initial version
        } else {
            bool ok = false;
            int schemaVersion = settingsValue.toInt(&ok);
            VERIFY_OR_DEBUG_ASSERT(ok && (schemaVersion >= 0)) {
                kLogger.critical()
                        << "Invalid database schema version" << settingsValue;
            }
            return schemaVersion;
        }
    }
}

SchemaManager::SchemaManager(const QSqlDatabase& database)
    : m_database(database),
      m_settingsDao(database),
      m_currentVersion(readCurrentSchemaVersion(m_settingsDao)) {
}

bool SchemaManager::isBackwardsCompatibleWithVersion(int targetVersion) const {
    QString backwardsCompatibleVersion =
            m_settingsDao.getValue(SETTINGS_MINCOMPATIBLE_STRING);
    bool ok = false;
    int iBackwardsCompatibleVersion = backwardsCompatibleVersion.toInt(&ok);

    // If the current backwards compatible schema version is not stored in the
    // settings table, assume the current schema version is only backwards
    // compatible with itself.
    if (backwardsCompatibleVersion.isNull() || !ok) {
        // rryan 11/2010 We just added the backwards compatible flags, and some
        // people using the Mixxx trunk are already on schema version 7. This
        // special case is for them. Schema version 7 is backwards compatible
        // with schema version 3.
        if (m_currentVersion == 7) {
            iBackwardsCompatibleVersion = 3;
        } else {
            iBackwardsCompatibleVersion = m_currentVersion;
        }
    }

    // If the target version is greater than the minimum compatible version of
    // the current schema, then the current schema is backwards compatible with
    // targetVersion.
    return iBackwardsCompatibleVersion <= targetVersion;
}

SchemaManager::Result SchemaManager::upgradeToSchemaVersion(
        const QString& schemaFilename,
        int targetVersion) {
    VERIFY_OR_DEBUG_ASSERT(m_currentVersion >= 0) {
        return Result::UpgradeFailed;
    }

    if (m_currentVersion == targetVersion) {
        kLogger.info()
                << "Database schema is up-to-date"
                << "at version" << m_currentVersion;
        return Result::CurrentVersion;
    } else if (m_currentVersion < targetVersion) {
        kLogger.info()
                << "Upgrading database schema"
                << "from version" << m_currentVersion
                << "to version" << targetVersion;
    } else {
        if (isBackwardsCompatibleWithVersion(targetVersion)) {
            kLogger.info()
                    << "Current database schema is newer"
                    << "at version" << m_currentVersion
                    << "and backwards compatible"
                    << "with version" << targetVersion;
            return Result::NewerVersionBackwardsCompatible;
        } else {
            kLogger.warning()
                    << "Current database schema is newer"
                    << "at version" << m_currentVersion
                    << "and incompatible"
                    << "with version" << targetVersion;
            return Result::NewerVersionIncompatible;
        }
    }

    if (kLogger.debugEnabled()) {
        kLogger.debug()
                << "Loading database schema migrations from"
                << schemaFilename;
    }
    QDomElement schemaRoot = XmlParse::openXMLFile(schemaFilename, "schema");
    if (schemaRoot.isNull()) {
        kLogger.critical()
                << "Failed to load database schema migrations from"
                << schemaFilename;
        return Result::SchemaError;
    }

    QDomNodeList revisions = schemaRoot.childNodes();

    QMap<int, QDomElement> revisionMap;

    for (int i = 0; i < revisions.count(); i++) {
        QDomElement revision = revisions.at(i).toElement();
        QString version = revision.attribute("version");
        VERIFY_OR_DEBUG_ASSERT(!version.isNull()) {
            kLogger.critical()
                    << "Failed to parse database schema migrations from"
                    << schemaFilename;
            return Result::SchemaError;
        }
        int iVersion = version.toInt();
        revisionMap[iVersion] = revision;
    }

    // The checks above guarantee that currentVersion < targetVersion when we
    // get here.
    while (m_currentVersion < targetVersion) {
        int nextVersion = m_currentVersion + 1;

        // Now that we bake the schema.xml into the binary it is a programming
        // error if we include a schema.xml that does not have information on
        // how to get all the way to targetVersion.
        VERIFY_OR_DEBUG_ASSERT(revisionMap.contains(nextVersion)) {
            kLogger.critical()
                     << "Migration path for upgrading database schema"
                     << "from version" << m_currentVersion
                     << "to version" << nextVersion
                     << "is missing";
            return Result::SchemaError;
        }

        QDomElement revision = revisionMap[nextVersion];
        QDomElement eDescription = revision.firstChildElement("description");
        QDomElement eSql = revision.firstChildElement("sql");
        QString minCompatibleVersion = revision.attribute("min_compatible");

        // Default the min-compatible version to the current version string if
        // it's not in the schema.xml
        if (minCompatibleVersion.isNull()) {
            minCompatibleVersion = QString::number(nextVersion);
        }

        VERIFY_OR_DEBUG_ASSERT(!eSql.isNull()) {
            kLogger.critical()
                    << "Failed to parse database schema migrations from"
                    << schemaFilename;
            return Result::SchemaError;
        }

        QString description = eDescription.text();
        QString sql = eSql.text();

        kLogger.info()
                << "Upgrading to database schema to version"
                << nextVersion << ":"
                << description.trimmed();

        SqlTransaction transaction(m_database);

        // TODO(XXX) We can't have semicolons in schema.xml for anything other
        // than statement separators.
        QStringList sqlStatements = sql.split(";");

        QStringListIterator it(sqlStatements);

        bool result = true;
        while (result && it.hasNext()) {
            QString statement = it.next().trimmed();
            if (statement.isEmpty()) {
                // skip blank lines
                continue;
            }
            FwdSqlQuery query(m_database, statement);
            result = query.isPrepared() && query.execPrepared();
            if (!result &&
                    query.hasError() &&
                    query.lastError().databaseText().startsWith(
                            QStringLiteral("duplicate column name: "))) {
                // New columns may have already been added during a previous
                // migration to a different (= preceding) schema version. This
                // is a very common situation during development when switching
                // between schema versions. Since SQLite does not allow to add
                // new columns only if they do not yet exist we need to account
                // for and handle those errors here after they occurred. If the
                // remaining migration finishes without other errors this is
                // probably ok.
                kLogger.warning()
                        << "Ignoring failed statement"
                        << statement
                        << "and continuing with schema migration";
                result = true;
            }
        }

        if (result) {
            m_settingsDao.setValue(SETTINGS_VERSION_STRING, nextVersion);
            m_settingsDao.setValue(SETTINGS_MINCOMPATIBLE_STRING, minCompatibleVersion);
            transaction.commit();
            m_currentVersion = nextVersion;
            kLogger.info()
                    << "Upgraded database schema"
                    << "to version" << m_currentVersion;
        } else {
            kLogger.critical()
                    << "Failed to upgrade database schema from version"
                    << m_currentVersion << "to version" << nextVersion;
            transaction.rollback();
            return Result::UpgradeFailed;
        }
    }
    return Result::UpgradeSucceeded;
}
