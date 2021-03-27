#include "database/schemamanager.h"

#include "util/assert.h"
#include "util/db/fwdsqlquery.h"
#include "util/db/sqltransaction.h"
#include "util/logger.h"
#include "util/math.h"
#include "util/optional.h"
#include "util/xml.h"

namespace {
mixxx::Logger kLogger("SchemaManager");

constexpr int INITIAL_VERSION = 0;

// Reapplying previous schema migrations is only supported starting
// with version 35. All following schema migrations must support to
// be reapplied without any data loss.
constexpr int MIN_REAPPLY_VERSION = 35;

const QString SETTINGS_VERSION_KEY = QStringLiteral("mixxx.schema.version");
const QString SETTINGS_LASTUSED_VERSION_KEY = QStringLiteral("mixxx.schema.last_used_version");
const QString SETTINGS_MINCOMPATIBLE_KEY = QStringLiteral("mixxx.schema.min_compatible_version");

std::optional<int> readSchemaVersion(
        const SettingsDAO& settings,
        const QString& key) {
    QString settingsValue = settings.getValue(key);
    // May be a null string if the schema has not been created yet.
    if (settingsValue.isNull()) {
        return std::nullopt;
    }
    bool ok = false;
    int schemaVersion = settingsValue.toInt(&ok);
    VERIFY_OR_DEBUG_ASSERT(ok && (schemaVersion >= INITIAL_VERSION)) {
        kLogger.critical()
                << "Invalid database schema version"
                << settingsValue;
        return std::nullopt;
    }
    return schemaVersion;
}

} // namespace

SchemaManager::SchemaManager(const QSqlDatabase& database)
        : m_settingsDao(database) {
}

int SchemaManager::readCurrentVersion() const {
    return readSchemaVersion(
            m_settingsDao,
            SETTINGS_VERSION_KEY)
            .value_or(INITIAL_VERSION);
}

int SchemaManager::readLastUsedVersion() const {
    const auto lastUsedVersion = readSchemaVersion(
            m_settingsDao,
            SETTINGS_LASTUSED_VERSION_KEY);
    if (lastUsedVersion) {
        return *lastUsedVersion;
    }
    // Fallback
    return readCurrentVersion();
}

int SchemaManager::readMinBackwardsCompatibleVersion() const {
    const auto minBackwardsCompatibleVersion = readSchemaVersion(
            m_settingsDao,
            SETTINGS_MINCOMPATIBLE_KEY);
    if (minBackwardsCompatibleVersion) {
        return *minBackwardsCompatibleVersion;
    }
    const auto currentVersion = readCurrentVersion();
    if (currentVersion <= 7) {
        // rryan 11/2010 We just added the backwards compatible flags, and some
        // people using the Mixxx trunk are already on schema version 7. This
        // special case is for them. Schema version 7 is backwards compatible
        // with schema version 3.
        return math_min(currentVersion, 3);
    }
    // If the current backwards compatible schema version is not stored in the
    // settings table, assume the current schema version is only backwards
    // compatible with itself.
    return currentVersion;
}

SchemaManager::Result SchemaManager::upgradeToSchemaVersion(
        int targetVersion,
        const QString& schemaFilename) {
    auto currentVersion = readCurrentVersion();
    VERIFY_OR_DEBUG_ASSERT(currentVersion >= INITIAL_VERSION) {
        return Result::UpgradeFailed;
    }

    const int lastUsedVersion = readLastUsedVersion();
    VERIFY_OR_DEBUG_ASSERT(lastUsedVersion >= INITIAL_VERSION) {
        return Result::UpgradeFailed;
    }
    VERIFY_OR_DEBUG_ASSERT(lastUsedVersion <= currentVersion) {
        // Fix inconsistent value
        m_settingsDao.setValue(
                SETTINGS_LASTUSED_VERSION_KEY,
                currentVersion);
    }

    if (targetVersion == currentVersion) {
        if (lastUsedVersion >= targetVersion) {
            if (lastUsedVersion > targetVersion) {
                m_settingsDao.setValue(
                        SETTINGS_LASTUSED_VERSION_KEY,
                        targetVersion);
            }
            kLogger.info()
                    << "Database schema is up-to-date"
                    << "at version" << currentVersion;
            return Result::CurrentVersion;
        }
        // Otherwise reapply migrations between lastUsedVersion
        // and targetVersion.
        DEBUG_ASSERT(lastUsedVersion < targetVersion);
    } else if (targetVersion < currentVersion) {
        if (targetVersion < readMinBackwardsCompatibleVersion()) {
            kLogger.warning()
                    << "Current database schema is newer"
                    << "at version" << currentVersion
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

    if (currentVersion < targetVersion) {
        kLogger.info()
                << "Upgrading database schema"
                << "from version" << currentVersion
                << "to version" << targetVersion;
    }
    int nextVersion = lastUsedVersion;
    while (nextVersion < targetVersion) {
        nextVersion += 1;
        VERIFY_OR_DEBUG_ASSERT(revisionMap.contains(nextVersion)) {
            kLogger.critical()
                    << "Migration path for upgrading database schema"
                    << "from version" << currentVersion
                    << "to version" << nextVersion
                    << "is missing";
            return Result::SchemaError;
        }

        if (nextVersion < currentVersion) {
            if (nextVersion < MIN_REAPPLY_VERSION) {
                kLogger.debug()
                        << "Migration to version"
                        << nextVersion
                        << "cannot be reapplied";
                continue;
            } else {
                kLogger.info()
                        << "Reapplying database schema migration"
                        << "to version"
                        << nextVersion;
            }
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
                << "Upgrading database schema to version"
                << nextVersion << ":"
                << description.trimmed();

        SqlTransaction transaction(m_settingsDao.database());

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
            FwdSqlQuery query(m_settingsDao.database(), statement);
            result = query.isPrepared() && query.execPrepared();
            if (!result &&
                    query.hasError() &&
                    query.lastError().databaseText().startsWith(
                            QStringLiteral("duplicate column name: "))) {
                // New columns may have already been added during a previous
                // migration to a different (= preceding) schema version. This
                // is a very common situation during development when switching
                // between schema versions. Since SQLite only allows to add new
                // columns if they do not yet exist, we need to account for and
                // handle those errors here after they occurred.
                // If the remaining migration finishes without other errors this
                // is probably ok.
                kLogger.warning()
                        << "Ignoring failed statement"
                        << statement
                        << "and continuing with schema migration";
                result = true;
            }
        }

        if (result) {
            if (nextVersion > currentVersion) {
                currentVersion = nextVersion;
                m_settingsDao.setValue(
                        SETTINGS_VERSION_KEY,
                        currentVersion);
                m_settingsDao.setValue(
                        SETTINGS_LASTUSED_VERSION_KEY,
                        currentVersion);
                m_settingsDao.setValue(
                        SETTINGS_MINCOMPATIBLE_KEY,
                        minCompatibleVersion);
                kLogger.info()
                        << "Upgraded database schema"
                        << "to version" << nextVersion;
            } else {
                kLogger.info()
                        << "Reapplied database schema migration"
                        << "to version" << nextVersion;
            }
            transaction.commit();
        } else {
            kLogger.critical()
                    << "Failed to upgrade database schema from version"
                    << currentVersion << "to version" << nextVersion;
            transaction.rollback();
            return Result::UpgradeFailed;
        }
    }

    if (targetVersion != lastUsedVersion) {
        m_settingsDao.setValue(
                SETTINGS_LASTUSED_VERSION_KEY,
                targetVersion);
    }
    if (targetVersion < currentVersion) {
        kLogger.info()
                << "Current database schema is newer"
                << "at version" << currentVersion
                << "and backwards compatible"
                << "with version" << targetVersion;
        return Result::NewerVersionBackwardsCompatible;
    } else {
        DEBUG_ASSERT(targetVersion == currentVersion);
        return Result::UpgradeSucceeded;
    }
}
