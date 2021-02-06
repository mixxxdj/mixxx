#pragma once

#include <QSqlDatabase>

#include "preferences/usersettings.h"
#include "library/dao/settingsdao.h"

/// The SchemaManager reads the database schema from the schemaFile
/// (res/schema.xml) and is responsible for checking compatibility as well as
/// upgrading the database if necessary.
/// It also caches some information about the current version in a SettingsDAO.
/// Note: If a version has no min_compatible information, it is assumed to have
/// no backwards compatibility.
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
    Result upgradeToSchemaVersion(int targetVersion, const QString& schemaFilename);

  private:
    const SettingsDAO m_settingsDao;
};
