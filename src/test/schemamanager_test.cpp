#include "database/schemamanager.h"

#include <QSqlQuery>

#include "library/dao/settingsdao.h"
#include "test/mixxxdbtest.h"

class SchemaManagerTest : public MixxxDbTest {};

TEST_F(SchemaManagerTest, CanUpgradeFreshDatabaseToRequiredVersion) {
    // Initial upgrade
    {
        SchemaManager schemaManager(dbConnection());
        SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
                MixxxDb::kRequiredSchemaVersion, MixxxDb::kDefaultSchemaFile);
        EXPECT_EQ(SchemaManager::Result::UpgradeSucceeded, result);
    }
    // Subsequent upgrade
    {
        SchemaManager schemaManager(dbConnection());
        SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
                MixxxDb::kRequiredSchemaVersion, MixxxDb::kDefaultSchemaFile);
        EXPECT_EQ(SchemaManager::Result::CurrentVersion, result);
    }
}

TEST_F(SchemaManagerTest, NonExistentSchema) {
    SchemaManager schemaManager(dbConnection());
    SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
            MixxxDb::kRequiredSchemaVersion, ":file_doesnt_exist.xml");
    EXPECT_EQ(SchemaManager::Result::SchemaError, result);
}

TEST_F(SchemaManagerTest, BackwardsCompatibleVersion) {
    // Establish preconditions for test
    {
        SchemaManager schemaManager(dbConnection());
        SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
                MixxxDb::kRequiredSchemaVersion, MixxxDb::kDefaultSchemaFile);
        ASSERT_EQ(SchemaManager::Result::UpgradeSucceeded, result);

        // Pretend the database version is one past the required version but
        // min_compatible is the required version.
        SettingsDAO settings(dbConnection());
        ASSERT_TRUE(settings.setValue(SchemaManager::SETTINGS_VERSION_STRING,
                MixxxDb::kRequiredSchemaVersion + 1));
        ASSERT_TRUE(settings.setValue(SchemaManager::SETTINGS_MINCOMPATIBLE_STRING,
                MixxxDb::kRequiredSchemaVersion));
    }

    SchemaManager schemaManager(dbConnection());
    SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
            MixxxDb::kRequiredSchemaVersion, MixxxDb::kDefaultSchemaFile);
    EXPECT_EQ(SchemaManager::Result::NewerVersionBackwardsCompatible, result);
}

TEST_F(SchemaManagerTest, BackwardsIncompatibleVersion) {
    // Establish preconditions for test
    {
        SchemaManager schemaManager(dbConnection());
        SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
                MixxxDb::kRequiredSchemaVersion, MixxxDb::kDefaultSchemaFile);
        ASSERT_EQ(SchemaManager::Result::UpgradeSucceeded, result);

        // Pretend the database version is one past the required version and
        // min_compatible is one past the required version.
        SettingsDAO settings(dbConnection());
        ASSERT_TRUE(settings.setValue(SchemaManager::SETTINGS_VERSION_STRING,
                MixxxDb::kRequiredSchemaVersion + 1));
        ASSERT_TRUE(settings.setValue(SchemaManager::SETTINGS_MINCOMPATIBLE_STRING,
                MixxxDb::kRequiredSchemaVersion + 1));
    }

    SchemaManager schemaManager(dbConnection());
    SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
            MixxxDb::kRequiredSchemaVersion, MixxxDb::kDefaultSchemaFile);
    EXPECT_EQ(SchemaManager::Result::NewerVersionIncompatible, result);
}

TEST_F(SchemaManagerTest, IgnoreDuplicateColumn) {
    // Establish preconditions for test
    {
        // Upgrade to version 3 to get the modern library table.
        SchemaManager schemaManager(dbConnection());
        SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
                3, MixxxDb::kDefaultSchemaFile);
        ASSERT_EQ(SchemaManager::Result::UpgradeSucceeded, result);

        // Add a column that will be added again in version 24.
        QSqlQuery query(dbConnection());
        ASSERT_TRUE(
                query.exec("ALTER TABLE library ADD COLUMN coverart_source TEXT"));
    }

    SchemaManager schemaManager(dbConnection());
    SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
            MixxxDb::kRequiredSchemaVersion, MixxxDb::kDefaultSchemaFile);
    EXPECT_EQ(SchemaManager::Result::UpgradeSucceeded, result);
}

TEST_F(SchemaManagerTest, UpgradeFailed) {
    // Establish preconditions for test
    {
        // Upgrade to version 3 to get the modern library table.
        SchemaManager schemaManager(dbConnection());
        SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
                3, MixxxDb::kDefaultSchemaFile);
        ASSERT_EQ(SchemaManager::Result::UpgradeSucceeded, result);

        // Drop a table that is expected to exist.
        QSqlQuery query(dbConnection());
        ASSERT_TRUE(query.exec("DROP TABLE PlaylistTracks"));
    }

    SchemaManager schemaManager(dbConnection());
    SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
            MixxxDb::kRequiredSchemaVersion, MixxxDb::kDefaultSchemaFile);
    EXPECT_EQ(SchemaManager::Result::UpgradeFailed, result);
}
