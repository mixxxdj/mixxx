#pragma once

#include <QSqlDatabase>

#include "preferences/usersettings.h"
#include "library/dao/settingsdao.h"

/// The SchemaManager reads the database schema from the schemaFile
/// (res/schema.xml in development) and is responsible for checking
/// compatibility as well as upgrading the database if necessary.
/// It also caches some information about the current version in a SettingsDAO.
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

    /// Check if the current version is backwards compatible with the given targetVersion.
    ///
    /// If the current version has no backwards compatibility info, it is
    /// assumed that it is only compatible with itself.
    ///
    /// Returns true if the targetVersion is greater or equal to the
    /// minimum compatible version of the current schema
    bool isBackwardsCompatibleWithVersion(int targetVersion) const;

    /// Tries to upgrade the database schema to the given targetVersion.
    /// First checks for version compatibility and whether the current version
    /// already suffices the target and potentially aborts with the appropriate Result.
    Result upgradeToSchemaVersion(
            const QString& schemaFilename,
            int targetVersion);

  private:
    QSqlDatabase m_database;
    SettingsDAO m_settingsDao;

    int m_currentVersion;
};
