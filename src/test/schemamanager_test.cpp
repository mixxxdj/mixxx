#include <gtest/gtest.h>

#include "test/mixxxtest.h"

#include "repository/repository.h"
#include "repository/schemamanager.h"

#include "library/dao/settingsdao.h"

#include "util/assert.h"


class SchemaManagerTest : public MixxxTest {
  protected:
    SchemaManagerTest()
            : m_repository(config()),
              m_dbConnectionScope(m_repository.dbConnectionPool()) {
    }

    const mixxx::DbConnectionPool::ThreadLocalScope& dbConnectionScope() const {
        return m_dbConnectionScope;
    }

  private:
    mixxx::Repository m_repository;
    const mixxx::DbConnectionPool::ThreadLocalScope m_dbConnectionScope;
};

TEST_F(SchemaManagerTest, CanUpgradeFreshDatabaseToRequiredVersion) {
    // Initial upgrade
    {
        SchemaManager schemaManager(dbConnectionScope());
        SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
                mixxx::Repository::kDefaultSchemaFile, mixxx::Repository::kRequiredSchemaVersion);
        EXPECT_EQ(SchemaManager::Result::UpgradeSucceeded, result);
    }
    // Subsequent upgrade(s)
    {
        SchemaManager schemaManager(dbConnectionScope());
        SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
                mixxx::Repository::kDefaultSchemaFile, mixxx::Repository::kRequiredSchemaVersion);
        EXPECT_EQ(SchemaManager::Result::CurrentVersion, result);
    }
}

TEST_F(SchemaManagerTest, NonExistentSchema) {
    SchemaManager schemaManager(dbConnectionScope());
    SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
            ":file_doesnt_exist.xml", mixxx::Repository::kRequiredSchemaVersion);
    EXPECT_EQ(SchemaManager::Result::SchemaError, result);
}

TEST_F(SchemaManagerTest, BackwardsCompatibleVersion) {
    // Establish preconditions for test
    {
        // Upgrade to version 1 to get the settings table.
        SchemaManager schemaManager(dbConnectionScope());
        SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
                mixxx::Repository::kDefaultSchemaFile, 1);
        EXPECT_EQ(SchemaManager::Result::UpgradeSucceeded, result);

        SettingsDAO settings(dbConnectionScope());
        settings.initialize();

        // Pretend the database version is one past the required version but
        // min_compatible is the required version.
        settings.setValue(SchemaManager::SETTINGS_VERSION_STRING,
                          mixxx::Repository::kRequiredSchemaVersion + 1);
        settings.setValue(SchemaManager::SETTINGS_MINCOMPATIBLE_STRING,
                          mixxx::Repository::kRequiredSchemaVersion);
    }

    SchemaManager schemaManager(dbConnectionScope());
    SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
            mixxx::Repository::kDefaultSchemaFile, mixxx::Repository::kRequiredSchemaVersion);
    EXPECT_EQ(SchemaManager::Result::NewerVersionBackwardsCompatible, result);
}

TEST_F(SchemaManagerTest, BackwardsIncompatibleVersion) {
    // Establish preconditions for test
    {
        // Upgrade to version 1 to get the settings table.
        SchemaManager schemaManager(dbConnectionScope());
        SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
                mixxx::Repository::kDefaultSchemaFile, 1);
        EXPECT_EQ(SchemaManager::Result::UpgradeSucceeded, result);

        SettingsDAO settings(dbConnectionScope());
        settings.initialize();

        // Pretend the database version is one past the required version and
        // min_compatible is one past the required version.
        settings.setValue(SchemaManager::SETTINGS_VERSION_STRING,
                          mixxx::Repository::kRequiredSchemaVersion + 1);
        settings.setValue(SchemaManager::SETTINGS_MINCOMPATIBLE_STRING,
                          mixxx::Repository::kRequiredSchemaVersion + 1);
    }

    SchemaManager schemaManager(dbConnectionScope());
    SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
            mixxx::Repository::kDefaultSchemaFile, mixxx::Repository::kRequiredSchemaVersion);
    EXPECT_EQ(SchemaManager::Result::NewerVersionIncompatible, result);
}

TEST_F(SchemaManagerTest, FailedUpgrade) {
    // Establish preconditions for test
    {
        // Upgrade to version 3 to get the modern library table.
        SchemaManager schemaManager(dbConnectionScope());
        SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
                mixxx::Repository::kDefaultSchemaFile, 3);
        EXPECT_EQ(SchemaManager::Result::UpgradeSucceeded, result);
    }

    // Add a column that is added in verison 24.
    QSqlQuery query(dbConnectionScope());
    EXPECT_TRUE(query.exec(
            "ALTER TABLE library ADD COLUMN coverart_source TEXT"));

    SchemaManager schemaManager(dbConnectionScope());
    SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
            mixxx::Repository::kDefaultSchemaFile, mixxx::Repository::kRequiredSchemaVersion);
    EXPECT_EQ(SchemaManager::Result::UpgradeFailed, result);
}
