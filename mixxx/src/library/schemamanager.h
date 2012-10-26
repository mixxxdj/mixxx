// schemamanager.h
// Created 12/29/2009 by RJ Ryan (rryan@mit.edu)

#ifndef SCHEMAMANAGER_H
#define SCHEMAMANAGER_H

#include <QtSql>

#include "configobject.h"
#include "library/dao/settingsdao.h"

class SchemaManager {
  public:
    static int upgradeToSchemaVersion(const QString& schemaFilename,
                                       QSqlDatabase& db, int targetVersion);
  private:
    static bool isBackwardsCompatible(SettingsDAO& settings,
                                      int currentVersion,
                                      int targetVersion);
    static int getCurrentSchemaVersion(SettingsDAO& settings);

    static const QString SETTINGS_VERSION_STRING;
    static const QString SETTINGS_MINCOMPATIBLE_STRING;
};

#endif /* SCHEMAMANAGER_H */
