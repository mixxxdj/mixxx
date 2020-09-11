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
    static const QString SETTINGS_VERSION_STRING;
    static const QString SETTINGS_MINCOMPATIBLE_STRING;

    enum class Result {
        CurrentVersion,
        NewerVersionBackwardsCompatible,
        NewerVersionIncompatible,
        UpgradeSucceeded,
        UpgradeFailed,
        SchemaError
    };

    explicit SchemaManager(const QSqlDatabase& database);

    int getCurrentVersion() const {
        return m_currentVersion;
    }

    /// Checks if the current schema version is backwards compatible with the
    /// given targetVersion.
    bool isBackwardsCompatibleWithVersion(int targetVersion) const;

    /// Tries to update the database schema to targetVersion.
    /// Pending changes are rolled back upon failure.
    /// No-op if the versions are incompatible or the targetVersion is older.
    Result upgradeToSchemaVersion(int targetVersion, const QString& schemaFilename);
  
  private:
    const QSqlDatabase m_database;
    const SettingsDAO m_settingsDao;

    int m_currentVersion;
};
