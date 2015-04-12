// schemamanager.h
// Created 12/29/2009 by RJ Ryan (rryan@mit.edu)

#ifndef SCHEMAMANAGER_H
#define SCHEMAMANAGER_H

#include <QtSql>

#include "configobject.h"
#include "library/dao/settingsdao.h"

class SchemaManager {
  public:
    enum Result {
        RESULT_OK,
        RESULT_BACKWARDS_INCOMPATIBLE,
        RESULT_UPGRADE_FAILED,
        RESULT_SCHEMA_ERROR
    };

    static Result upgradeToSchemaVersion(const QString& schemaFilename,
                                         QSqlDatabase& db, const int targetVersion);

    static const QString SETTINGS_VERSION_STRING;
    static const QString SETTINGS_MINCOMPATIBLE_STRING;

  private:
    static bool isBackwardsCompatible(SettingsDAO& settings,
                                      int currentVersion,
                                      int targetVersion);
    static int getCurrentSchemaVersion(SettingsDAO& settings);
};

#endif /* SCHEMAMANAGER_H */
