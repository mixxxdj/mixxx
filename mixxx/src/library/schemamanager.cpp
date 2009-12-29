// schemamanager.cpp
// Created 12/29/2009 by RJ Ryan (rryan@mit.edu)

#include <QtCore>
#include <QtDebug>

#include "library/schemamanager.h"
#include "library/dao/settingsdao.h"
#include "widget/wwidget.h"

// static
bool SchemaManager::upgradeToSchemaVersion(ConfigObject<ConfigValue>* config,
                                           QSqlDatabase& db, int targetVersion) {

    SettingsDAO settings(db);
    QString currentSchemaVersion = settings.getValue("mixxx.schema.version");

    // May be a null string if the schema has not been created. We default the
    // startVersion to 0 so that we automatically try to upgrade to revision 1.
    int currentVersion = 0;
    if (!currentSchemaVersion.isNull()) {
        currentVersion = currentSchemaVersion.toInt();
    }

    Q_ASSERT(currentVersion >= 0);

    if (currentVersion == targetVersion) {
        qDebug() << "SchemaManager::upgradeToSchemaVersion already at version"
                 << targetVersion;
        return true;
    } else if (currentVersion < targetVersion) {
        qDebug() << "SchemaManager::upgradeToSchemaVersion upgrading"
                 << targetVersion-currentVersion << "versions to version"
                 << targetVersion;
    } else {
        qDebug() << "SchemaManager::upgradeToSchemaVersion already past target "
                "version. currentVersion:"
                 << currentVersion << "targetVersion:"
                 << targetVersion;

    }


    QString schemaFilename = config->getConfigPath();
    schemaFilename.append("schema.xml");
    qDebug() << "Loading schema" << schemaFilename;
    QDomElement schemaRoot = WWidget::openXMLFile(schemaFilename, "schema");

    QDomNodeList revisions = schemaRoot.childNodes();

    QMap<int, QDomElement> revisionMap;

    for (int i = 0; i < revisions.count(); i++) {
        QDomElement revision = revisions.at(i).toElement();
        QString version = revision.attribute("version");
        Q_ASSERT(!version.isNull());
        int iVersion = version.toInt();
        revisionMap[iVersion] = revision;
    }

    bool success = true;

    while (currentVersion != targetVersion) {
        int thisTarget;
        if (currentVersion > targetVersion) {
            thisTarget = currentVersion - 1;
            qDebug() << "Downgrade not yet supported.";
            success = false;
            break;
        } else {
            thisTarget = currentVersion + 1;
        }

        if (!revisionMap.contains(thisTarget)) {
            qDebug() << "SchemaManager::upgradeToSchemaVersion"
                     << "Don't know how to get to"
                     << thisTarget << "from" << currentVersion;
            success = false;
            break;
        }

        QDomElement revision = revisionMap[thisTarget];
        QDomElement eDescription = revision.firstChildElement("description");
        QDomElement eSql = revision.firstChildElement("sql");

        Q_ASSERT(!eDescription.isNull() && !eSql.isNull());

        QString description = eDescription.text();
        QString sql = eSql.text();


        qDebug() << "Applying version" << thisTarget << ":"
                 << description.trimmed();

        db.transaction();

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
            settings.setValue("mixxx.schema.version", thisTarget);
            db.commit();
        } else {
            success = false;
            qDebug() << "Failed to move from version" << currentVersion
                     << "to version" << thisTarget;
            db.rollback();
            break;
        }
    }

    return success;
}
