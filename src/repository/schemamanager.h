#ifndef SCHEMAMANAGER_H
#define SCHEMAMANAGER_H

#include <QSqlDatabase>

#include "preferences/usersettings.h"
#include "library/dao/settingsdao.h"

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

    bool isBackwardsCompatibleWithVersion(int targetVersion) const;

    Result upgradeToSchemaVersion(
            const QString& schemaFilename,
            int targetVersion);

  private:
    QSqlDatabase m_database;
    SettingsDAO m_settingsDao;

    int m_currentVersion;
};

#endif /* SCHEMAMANAGER_H */
