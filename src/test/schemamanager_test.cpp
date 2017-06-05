#include <gtest/gtest.h>

#include "test/mixxxtest.h"

#include "database/schemamanager.h"
#include "database/mixxxdb.h"

#include "library/dao/settingsdao.h"

#include "util/assert.h"


class SchemaManagerTest : public MixxxTest {
  protected:
    SchemaManagerTest()
            : m_dbConnection(config()->getSettingsPath()) {
    }

    QSqlDatabase database() const {
        return m_dbConnection.database();
    }

  private:
    DbConnection m_dbConnection;
};

TEST_F(SchemaManagerTest, CanUpgradeFreshDatabaseToRequiredVersion) {
    // Initial upgrade
    {
        SchemaManager schemaManager(database());
        SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
                MixxxDB::kDefaultSchemaFile, MixxxDB::kRequiredSchemaVersion);
        EXPECT_EQ(SchemaManager::Result::UpgradeSucceeded, result);
    }
    // Subsequent upgrade(s)
    {
        SchemaManager schemaManager(database());
        SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
                MixxxDB::kDefaultSchemaFile, MixxxDB::kRequiredSchemaVersion);
        EXPECT_EQ(SchemaManager::Result::CurrentVersion, result);
    }
}

TEST_F(SchemaManagerTest, NonExistentSchema) {
    SchemaManager schemaManager(database());
    SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
            ":file_doesnt_exist.xml", MixxxDB::kRequiredSchemaVersion);
    EXPECT_EQ(SchemaManager::Result::SchemaError, result);
}

TEST_F(SchemaManagerTest, BackwardsCompatibleVersion) {
    // Establish preconditions for test
    {
        // Upgrade to version 1 to get the settings table.
        SchemaManager schemaManager(database());
        SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
                MixxxDB::kDefaultSchemaFile, 1);
        EXPECT_EQ(SchemaManager::Result::UpgradeSucceeded, result);

        SettingsDAO settings(database());
        settings.initialize();

        // Pretend the database version is one past the required version but
        // min_compatible is the required version.
        settings.setValue(SchemaManager::SETTINGS_VERSION_STRING,
                          MixxxDB::kRequiredSchemaVersion + 1);
        settings.setValue(SchemaManager::SETTINGS_MINCOMPATIBLE_STRING,
                          MixxxDB::kRequiredSchemaVersion);
    }

    SchemaManager schemaManager(database());
    SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
            MixxxDB::kDefaultSchemaFile, MixxxDB::kRequiredSchemaVersion);
    EXPECT_EQ(SchemaManager::Result::NewerVersionBackwardsCompatible, result);
}

TEST_F(SchemaManagerTest, BackwardsIncompatibleVersion) {
    // Establish preconditions for test
    {
        // Upgrade to version 1 to get the settings table.
        SchemaManager schemaManager(database());
        SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
                MixxxDB::kDefaultSchemaFile, 1);
        EXPECT_EQ(SchemaManager::Result::UpgradeSucceeded, result);

        SettingsDAO settings(database());
        settings.initialize();

        // Pretend the database version is one past the required version and
        // min_compatible is one past the required version.
        settings.setValue(SchemaManager::SETTINGS_VERSION_STRING,
                          MixxxDB::kRequiredSchemaVersion + 1);
        settings.setValue(SchemaManager::SETTINGS_MINCOMPATIBLE_STRING,
                          MixxxDB::kRequiredSchemaVersion + 1);
    }

    SchemaManager schemaManager(database());
    SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
            MixxxDB::kDefaultSchemaFile, MixxxDB::kRequiredSchemaVersion);
    EXPECT_EQ(SchemaManager::Result::NewerVersionIncompatible, result);
}

TEST_F(SchemaManagerTest, FailedUpgrade) {
    // Establish preconditions for test
    {
        // Upgrade to version 3 to get the modern library table.
        SchemaManager schemaManager(database());
        SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
                MixxxDB::kDefaultSchemaFile, 3);
        EXPECT_EQ(SchemaManager::Result::UpgradeSucceeded, result);
    }

    // Add a column that is added in verison 24.
    QSqlQuery query(database());
    EXPECT_TRUE(query.exec(
            "ALTER TABLE library ADD COLUMN coverart_source TEXT"));

    SchemaManager schemaManager(database());
    SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
            MixxxDB::kDefaultSchemaFile, MixxxDB::kRequiredSchemaVersion);
    EXPECT_EQ(SchemaManager::Result::UpgradeFailed, result);
}
