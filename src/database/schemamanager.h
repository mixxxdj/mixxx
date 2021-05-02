#ifndef SCHEMAMANAGER_H
#define SCHEMAMANAGER_H

#include <QSqlDatabase>

#include "preferences/usersettings.h"
#include "library/dao/settingsdao.h"

class SchemaManager {
  public:
    enum class Result {
        CurrentVersion,
        NewerVersionBackwardsCompatible,
        NewerVersionIncompatible,
        UpgradeSucceeded,
        UpgradeFailed,
        SchemaError
    };

    explicit SchemaManager(const QSqlDatabase& database);

    int readCurrentVersion() const;
    int readLastUsedVersion() const;
    int readMinBackwardsCompatibleVersion() const;

    /// Tries to update the database schema to targetVersion.
    /// Pending changes are rolled back upon failure.
    /// No-op if the versions are incompatible or the targetVersion is older.
    Result upgradeToSchemaVersion(
            int targetVersion,
            const QString& schemaFilename);

  private:
    const SettingsDAO m_settingsDao;
};

#endif /* SCHEMAMANAGER_H */
