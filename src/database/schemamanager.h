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

    /// If the current version has no backwards compatibility info, it is
    /// assumed that it is only compatible with itself.
    ///
    /// Returns true if the targetVersion is greater or equal to the
    /// minimum compatible version of the current schema
    bool isBackwardsCompatibleWithVersion(int targetVersion) const;

    /// Does nothing if the versions are incompatible or the targetVersion is
    /// older than the current one.
    Result upgradeToSchemaVersion(
            const QString& schemaFilename,
            int targetVersion);

  private:
    QSqlDatabase m_database;
    SettingsDAO m_settingsDao;

    int m_currentVersion;
};
