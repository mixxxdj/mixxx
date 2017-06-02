#include <gtest/gtest.h>

#include <QtDebug>

#include "test/mixxxtest.h"
#include "library/trackcollection.h"
#include "library/schemamanager.h"
#include "library/dao/settingsdao.h"
#include "util/assert.h"

class SchemaManagerTest : public MixxxTest {
  protected:
    SchemaManagerTest()
            : m_trackCollection(config()),
              m_db(m_trackCollection.database()) {
    }

    TrackCollection m_trackCollection;
    QSqlDatabase m_db;
};

TEST_F(SchemaManagerTest, CanUpgradeFreshDatabaseToRequiredVersion) {
    SchemaManager schemaManager(m_db);
    SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
            TrackCollection::kDefaultSchemaFile, TrackCollection::kRequiredSchemaVersion);
    EXPECT_EQ(SchemaManager::RESULT_OK, result);
}

TEST_F(SchemaManagerTest, NonExistentSchema) {
    SchemaManager schemaManager(m_db);
    SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
            ":file_doesnt_exist.xml", TrackCollection::kRequiredSchemaVersion);
    EXPECT_EQ(SchemaManager::RESULT_SCHEMA_ERROR, result);
}

TEST_F(SchemaManagerTest, BackwardsCompatibleVersion) {
    // Establish preconditions for test
    {
        // Upgrade to version 1 to get the settings table.
        SchemaManager schemaManager(m_db);
        SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
                TrackCollection::kDefaultSchemaFile, 1);
        EXPECT_EQ(SchemaManager::RESULT_OK, result);

        SettingsDAO settings(m_db);
        settings.initialize();

        // Pretend the database version is one past the required version but
        // min_compatible is the required version.
        settings.setValue(SchemaManager::SETTINGS_VERSION_STRING,
                          TrackCollection::kRequiredSchemaVersion + 1);
        settings.setValue(SchemaManager::SETTINGS_MINCOMPATIBLE_STRING,
                          TrackCollection::kRequiredSchemaVersion);
    }

    SchemaManager schemaManager(m_db);
    SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
            TrackCollection::kDefaultSchemaFile, TrackCollection::kRequiredSchemaVersion);
    EXPECT_EQ(SchemaManager::RESULT_OK, result);
}

TEST_F(SchemaManagerTest, BackwardsIncompatibleVersion) {
    // Establish preconditions for test
    {
        // Upgrade to version 1 to get the settings table.
        SchemaManager schemaManager(m_db);
        SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
                TrackCollection::kDefaultSchemaFile, 1);
        EXPECT_EQ(SchemaManager::RESULT_OK, result);

        SettingsDAO settings(m_db);
        settings.initialize();

        // Pretend the database version is one past the required version and
        // min_compatible is one past the required version.
        settings.setValue(SchemaManager::SETTINGS_VERSION_STRING,
                          TrackCollection::kRequiredSchemaVersion + 1);
        settings.setValue(SchemaManager::SETTINGS_MINCOMPATIBLE_STRING,
                          TrackCollection::kRequiredSchemaVersion + 1);
    }

    SchemaManager schemaManager(m_db);
    SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
            TrackCollection::kDefaultSchemaFile, TrackCollection::kRequiredSchemaVersion);
    EXPECT_EQ(SchemaManager::RESULT_BACKWARDS_INCOMPATIBLE, result);
}

TEST_F(SchemaManagerTest, FailedUpgrade) {
    // Establish preconditions for test
    {
        // Upgrade to version 3 to get the modern library table.
        SchemaManager schemaManager(m_db);
        SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
                TrackCollection::kDefaultSchemaFile, 3);
        EXPECT_EQ(SchemaManager::RESULT_OK, result);
    }

    // Add a column that is added in verison 24.
    QSqlQuery query(m_db);
    EXPECT_TRUE(query.exec(
            "ALTER TABLE library ADD COLUMN coverart_source TEXT"));

    SchemaManager schemaManager(m_db);
    SchemaManager::Result result = schemaManager.upgradeToSchemaVersion(
            TrackCollection::kDefaultSchemaFile, TrackCollection::kRequiredSchemaVersion);
    EXPECT_EQ(SchemaManager::RESULT_UPGRADE_FAILED, result);
}
