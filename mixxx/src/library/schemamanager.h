// schemamanager.h
// Created 12/29/2009 by RJ Ryan (rryan@mit.edu)

#ifndef SCHEMAMANAGER_H
#define SCHEMAMANAGER_H

#include <QtSql>

#include "configobject.h"

class SchemaManager {
  public:
    static bool upgradeToSchemaVersion(ConfigObject<ConfigValue>* config,
                                       QSqlDatabase& db, int targetVersion);
};

#endif /* SCHEMAMANAGER_H */
