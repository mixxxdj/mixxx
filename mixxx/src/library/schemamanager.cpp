// schemamanager.cpp
// Created 12/29/2009 by RJ Ryan (rryan@mit.edu)

#include <QtCore>
#include <QtDebug>

#include "library/schemamanager.h"
#include "library/queryutil.h"
#include "xmlparse.h"

const QString SchemaManager::SETTINGS_VERSION_STRING = "mixxx.schema.version";
const QString SchemaManager::SETTINGS_MINCOMPATIBLE_STRING = "mixxx.schema.min_compatible_version";

// static
int SchemaManager::upgradeToSchemaVersion(const QString& schemaFilename,
                                           QSqlDatabase& db, int targetVersion) {

    SettingsDAO settings(db);
    int currentVersion = getCurrentSchemaVersion(settings);
    Q_ASSERT(currentVersion >= 0);

    if (currentVersion == targetVersion) {
        qDebug() << "SchemaManager::upgradeToSchemaVersion already at version"
                 << targetVersion;
        return 0;
    } else if (currentVersion < targetVersion) {
        qDebug() << "SchemaManager::upgradeToSchemaVersion upgrading"
                 << targetVersion-currentVersion << "versions to version"
                 << targetVersion;
    } else {
        qDebug() << "SchemaManager::upgradeToSchemaVersion already past target "
                "version. currentVersion:"
                 << currentVersion << "targetVersion:"
                 << targetVersion;

        if (isBackwardsCompatible(settings, currentVersion, targetVersion)) {
            qDebug() << "Current schema version is backwards-compatible with" << targetVersion;
            return 0;
        }
    }

    qDebug() << "Loading schema" << schemaFilename;
    QDomElement schemaRoot = XmlParse::openXMLFile(schemaFilename, "schema");

    if (schemaRoot.isNull()) {
        // Error parsing xml file
        return -3;
    }

    QDomNodeList revisions = schemaRoot.childNodes();

    QMap<int, QDomElement> revisionMap;

    for (int i = 0; i < revisions.count(); i++) {
        QDomElement revision = revisions.at(i).toElement();
        QString version = revision.attribute("version");
        Q_ASSERT(!version.isNull());
        int iVersion = version.toInt();
        revisionMap[iVersion] = revision;
    }

    int success = 0;

    while (currentVersion != targetVersion) {
        int thisTarget;
        if (currentVersion > targetVersion) {
            thisTarget = currentVersion - 1;
            qDebug() << "Downgrade not yet supported.";
            success = -1;
            break;
        } else {
            thisTarget = currentVersion + 1;
        }

        if (!revisionMap.contains(thisTarget)) {
            qDebug() << "SchemaManager::upgradeToSchemaVersion"
                     << "Don't know how to get to"
                     << thisTarget << "from" << currentVersion;
            success = -1;
            break;
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

        Q_ASSERT(!eDescription.isNull() && !eSql.isNull());

        QString description = eDescription.text();
        QString sql = eSql.text();


        qDebug() << "Applying version" << thisTarget << ":"
                 << description.trimmed();

        ScopedTransaction transaction(db);

        // TODO(XXX) We can't have semicolons in schema.xml for anything other
        // than statement separators.
        QStringList sqlStatements = sql.split(";");

        QStringListIterator it(sqlStatements);

        QSqlQuery query(db);
        bool result = true;
        while (result && it.hasNext()) {
            QString statement = it.next().trimmed();
            if (statement.isEmpty()) {
                continue;
            }
            result = result && query.exec(statement);
            if (!result) {
                qDebug() << "Failed query:"
                         << statement
                         << query.lastError();
            }
        }

        if (result) {
            currentVersion = thisTarget;
            settings.setValue(SETTINGS_VERSION_STRING, thisTarget);
            settings.setValue(SETTINGS_MINCOMPATIBLE_STRING, minCompatibleVersion);
            transaction.commit();
        } else {
            success = -2;
            qDebug() << "Failed to move from version" << currentVersion
                     << "to version" << thisTarget;
            transaction.rollback();
            break;
        }
    }

    return success;
}

// static
int SchemaManager::getCurrentSchemaVersion(SettingsDAO& settings) {
    QString currentSchemaVersion = settings.getValue(SETTINGS_VERSION_STRING);
    // May be a null string if the schema has not been created. We default the
    // startVersion to 0 so that we automatically try to upgrade to revision 1.
    int currentVersion = 0;
    if (!currentSchemaVersion.isNull()) {
        currentVersion = currentSchemaVersion.toInt();
    }
    return currentVersion;
}

// static
bool SchemaManager::isBackwardsCompatible(SettingsDAO& settings,
                                          int currentVersion,
                                          int targetVersion) {
    QString backwardsCompatibleVersion =
            settings.getValue(SETTINGS_MINCOMPATIBLE_STRING);
    int iBackwardsCompatibleVersion = -1;

    // If the current backwards compatible schema version is not stored in the
    // settings table, assume the current schema version is only backwards
    // compatible with itself.
    if (backwardsCompatibleVersion.isNull()) {
        // rryan 11/2010 We just added the backwards compatible flags, and some
        // people using the Mixxx trunk are already on schema version 7. This
        // special case is for them. Schema version 7 is backwards compatible
        // with schema version 3.
        if (currentVersion == 7) {
            iBackwardsCompatibleVersion = 3;
        } else {
            iBackwardsCompatibleVersion = currentVersion;
        }
    }

    // If the target version is greater than the minimum compatible version of
    // the current schema, then the current schema is backwards compatible with
    // targetVersion.
    return iBackwardsCompatibleVersion <= targetVersion;
}
