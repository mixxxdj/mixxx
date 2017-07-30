#include <gtest/gtest.h>

#include "test/mixxxtest.h"

#include "database/mixxxdb.h"
#include "database/schemamanager.h"
#include "util/db/dbconnectionpooler.h"
#include "util/db/dbconnectionpooled.h"

#include "library/dao/settingsdao.h"

#include "util/assert.h"


class SchemaManagerTest : public MixxxTest {
  protected:
    SchemaManagerTest()
            : m_mixxxDb(config()),
              m_dbConnectionPooler(m_mixxxDb.connectionPool()),
              m_dbConnection(mixxx::DbConnectionPooled(m_mixxxDb.connectionPool())) {
    }

    QSqlDatabase dbConnection() const {
        return m_dbConnection;
    }

  private:
    const MixxxDb m_mixxxDb;
    const mixxx::DbConnectionPooler m_dbConnectionPooler;
    QSqlDatabase m_dbConnection;
};

TEST_F(SchemaManagerTest, CanUpgradeFreshDatabaseToRequiredVersion) {
    // Initial upgrade
    {
        SchemaManager schemaManager(dbConnection());
        SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
                MixxxDb::kDefaultSchemaFile, MixxxDb::kRequiredSchemaVersion);
        EXPECT_EQ(SchemaManager::Result::UpgradeSucceeded, result);
    }
    // Subsequent upgrade(s)
    {
        SchemaManager schemaManager(dbConnection());
        SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
                MixxxDb::kDefaultSchemaFile, MixxxDb::kRequiredSchemaVersion);
        EXPECT_EQ(SchemaManager::Result::CurrentVersion, result);
    }
}

TEST_F(SchemaManagerTest, NonExistentSchema) {
    SchemaManager schemaManager(dbConnection());
    SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
            ":file_doesnt_exist.xml", MixxxDb::kRequiredSchemaVersion);
    EXPECT_EQ(SchemaManager::Result::SchemaError, result);
}

TEST_F(SchemaManagerTest, BackwardsCompatibleVersion) {
    // Establish preconditions for test
    {
        // Upgrade to version 1 to get the settings table.
        SchemaManager schemaManager(dbConnection());
        SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
                MixxxDb::kDefaultSchemaFile, 1);
        EXPECT_EQ(SchemaManager::Result::UpgradeSucceeded, result);

        SettingsDAO settings(dbConnection());
        settings.initialize();

        // Pretend the database version is one past the required version but
        // min_compatible is the required version.
        settings.setValue(SchemaManager::SETTINGS_VERSION_STRING,
                          MixxxDb::kRequiredSchemaVersion + 1);
        settings.setValue(SchemaManager::SETTINGS_MINCOMPATIBLE_STRING,
                          MixxxDb::kRequiredSchemaVersion);
    }

    SchemaManager schemaManager(dbConnection());
    SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
            MixxxDb::kDefaultSchemaFile, MixxxDb::kRequiredSchemaVersion);
    EXPECT_EQ(SchemaManager::Result::NewerVersionBackwardsCompatible, result);
}

TEST_F(SchemaManagerTest, BackwardsIncompatibleVersion) {
    // Establish preconditions for test
    {
        // Upgrade to version 1 to get the settings table.
        SchemaManager schemaManager(dbConnection());
        SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
                MixxxDb::kDefaultSchemaFile, 1);
        EXPECT_EQ(SchemaManager::Result::UpgradeSucceeded, result);

        SettingsDAO settings(dbConnection());
        settings.initialize();

        // Pretend the database version is one past the required version and
        // min_compatible is one past the required version.
        settings.setValue(SchemaManager::SETTINGS_VERSION_STRING,
                          MixxxDb::kRequiredSchemaVersion + 1);
        settings.setValue(SchemaManager::SETTINGS_MINCOMPATIBLE_STRING,
                          MixxxDb::kRequiredSchemaVersion + 1);
    }

    SchemaManager schemaManager(dbConnection());
    SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
            MixxxDb::kDefaultSchemaFile, MixxxDb::kRequiredSchemaVersion);
    EXPECT_EQ(SchemaManager::Result::NewerVersionIncompatible, result);
}

TEST_F(SchemaManagerTest, FailedUpgrade) {
    // Establish preconditions for test
    {
        // Upgrade to version 3 to get the modern library table.
        SchemaManager schemaManager(dbConnection());
        SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
                MixxxDb::kDefaultSchemaFile, 3);
        EXPECT_EQ(SchemaManager::Result::UpgradeSucceeded, result);
    }

    // Add a column that is added in verison 24.
    QSqlQuery query(dbConnection());
    EXPECT_TRUE(query.exec(
            "ALTER TABLE library ADD COLUMN coverart_source TEXT"));

    SchemaManager schemaManager(dbConnection());
    SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
            MixxxDb::kDefaultSchemaFile, MixxxDb::kRequiredSchemaVersion);
    EXPECT_EQ(SchemaManager::Result::UpgradeFailed, result);
}
