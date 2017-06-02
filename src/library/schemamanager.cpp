// schemamanager.cpp
// Created 12/29/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "library/schemamanager.h"
#include "library/queryutil.h"
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
        return RESULT_UPGRADE_FAILED;
    }

    if (m_currentVersion == targetVersion) {
        kLogger.info()
                << "Current database schema version is"
                << m_currentVersion;
        return RESULT_OK;
    } else if (m_currentVersion < targetVersion) {
        kLogger.info()
                << "Upgrading database schema from version"
                << m_currentVersion
                << "to version"
                << targetVersion;
    } else {
        if (isBackwardsCompatibleWithVersion(targetVersion)) {
            kLogger.info()
                    << "Current database schema version is"
                    << m_currentVersion
                    << "and backwards compatible with version"
                    << targetVersion;
            return RESULT_OK;
        } else {
            kLogger.warning()
                    << "Current database schema version is"
                    << m_currentVersion
                    << "and incompatible with version"
                    << targetVersion;
            return RESULT_BACKWARDS_INCOMPATIBLE;
        }
    }

    kLogger.debug() << "Loading schema" << schemaFilename;
    QDomElement schemaRoot = XmlParse::openXMLFile(schemaFilename, "schema");

    if (schemaRoot.isNull()) {
        // Error parsing xml file
        return RESULT_SCHEMA_ERROR;
    }

    QDomNodeList revisions = schemaRoot.childNodes();

    QMap<int, QDomElement> revisionMap;

    for (int i = 0; i < revisions.count(); i++) {
        QDomElement revision = revisions.at(i).toElement();
        QString version = revision.attribute("version");
        VERIFY_OR_DEBUG_ASSERT(!version.isNull()) {
            // xml file is not valid
            return RESULT_SCHEMA_ERROR;
        }
        int iVersion = version.toInt();
        revisionMap[iVersion] = revision;
    }

    // The checks above guarantee that currentVersion < targetVersion when we
    // get here.
    while (m_currentVersion < targetVersion) {
        int thisTarget = m_currentVersion + 1;

        // Now that we bake the schema.xml into the binary it is a programming
        // error if we include a schema.xml that does not have information on
        // how to get all the way to targetVersion.
        if (!revisionMap.contains(thisTarget)) {
            kLogger.warning() << "upgradeToSchemaVersion"
                     << "Don't know how to get to"
                     << thisTarget << "from" << m_currentVersion;
            return RESULT_SCHEMA_ERROR;
        }

        QDomElement revision = revisionMap[thisTarget];
        QDomElement eDescription = revision.firstChildElement("description");
        QDomElement eSql = revision.firstChildElement("sql");
        QString minCompatibleVersion = revision.attribute("min_compatible");

        // Default the min-compatible version to the current version string if
        // it's not in the schema.xml
        if (minCompatibleVersion.isNull()) {
            minCompatibleVersion = QString::number(thisTarget);
        }

        VERIFY_OR_DEBUG_ASSERT(!eSql.isNull()) {
            // xml file is not valid
            return RESULT_SCHEMA_ERROR;
        }

        QString description = eDescription.text();
        QString sql = eSql.text();

        kLogger.debug() << "Applying version" << thisTarget << ":"
                 << description.trimmed();

        ScopedTransaction transaction(m_database);

        // TODO(XXX) We can't have semicolons in schema.xml for anything other
        // than statement separators.
        QStringList sqlStatements = sql.split(";");

        QStringListIterator it(sqlStatements);

        QSqlQuery query(m_database);
        bool result = true;
        while (result && it.hasNext()) {
            QString statement = it.next().trimmed();
            if (statement.isEmpty()) {
                continue;
            }
            result = result && query.exec(statement);
            if (!result) {
                kLogger.warning() << "Failed query:"
                         << statement
                         << query.lastError();
            }
        }

        if (result) {
            m_settingsDao.setValue(SETTINGS_VERSION_STRING, thisTarget);
            m_settingsDao.setValue(SETTINGS_MINCOMPATIBLE_STRING, minCompatibleVersion);
            transaction.commit();
            m_currentVersion = thisTarget;
        } else {
            kLogger.warning() << "Failed to move from version" << m_currentVersion
                     << "to version" << thisTarget;
            transaction.rollback();
            return RESULT_UPGRADE_FAILED;
        }
    }
    return RESULT_OK;
}
