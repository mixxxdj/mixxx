#include <gtest/gtest.h>

#include <QSqlDatabase>
#include <QTemporaryFile>
#include <QtDebug>

#include "test/mixxxtest.h"
#include "library/trackcollection.h"
#include "library/schemamanager.h"
#include "library/dao/settingsdao.h"

class SchemaManagerTest : public MixxxTest {
  protected:
    SchemaManagerTest()
            : m_dbFile("mixxxdb.sqlite") {
        Q_ASSERT(m_dbFile.open());
        m_db = QSqlDatabase::addDatabase("QSQLITE");
        m_db.setHostName("localhost");
        m_db.setUserName("mixxx");
        m_db.setPassword("mixxx");
        m_db.setDatabaseName(m_dbFile.fileName());
        Q_ASSERT(m_db.open());
    }

    QTemporaryFile m_dbFile;
    QSqlDatabase m_db;
};

TEST_F(SchemaManagerTest, CanUpgradeFreshDatabaseToRequiredVersion) {
    SchemaManager::Result result = SchemaManager::upgradeToSchemaVersion(
            ":schema.xml", m_db, TrackCollection::kRequiredSchemaVersion);
    EXPECT_EQ(SchemaManager::RESULT_OK, result);
}

TEST_F(SchemaManagerTest, NonExistentSchema) {
    SchemaManager::Result result = SchemaManager::upgradeToSchemaVersion(
            ":file_doesnt_exist.xml", m_db,
            TrackCollection::kRequiredSchemaVersion);
    EXPECT_EQ(SchemaManager::RESULT_SCHEMA_ERROR, result);
}

TEST_F(SchemaManagerTest, BackwardsCompatibleVersion) {
    // Upgrade to version 1 to get the settings table.
    SchemaManager::Result result = SchemaManager::upgradeToSchemaVersion(
            ":schema.xml", m_db, 1);
    EXPECT_EQ(SchemaManager::RESULT_OK, result);

    SettingsDAO settings(m_db);
    settings.initialize();

    // Pretend the database version is one past the required version but
    // min_compatible is the required version.
    settings.setValue(SchemaManager::SETTINGS_VERSION_STRING,
                      TrackCollection::kRequiredSchemaVersion + 1);
    settings.setValue(SchemaManager::SETTINGS_MINCOMPATIBLE_STRING,
                      TrackCollection::kRequiredSchemaVersion);

    result = SchemaManager::upgradeToSchemaVersion(
            ":schema.xml", m_db, TrackCollection::kRequiredSchemaVersion);
    EXPECT_EQ(SchemaManager::RESULT_OK, result);
}

TEST_F(SchemaManagerTest, BackwardsIncompatibleVersion) {
    // Upgrade to version 1 to get the settings table.
    SchemaManager::Result result = SchemaManager::upgradeToSchemaVersion(
            ":schema.xml", m_db, 1);
    EXPECT_EQ(SchemaManager::RESULT_OK, result);

    SettingsDAO settings(m_db);
    settings.initialize();

    // Pretend the database version is one past the required version and
    // min_compatible is one past the required version.
    settings.setValue(SchemaManager::SETTINGS_VERSION_STRING,
                      TrackCollection::kRequiredSchemaVersion + 1);
    settings.setValue(SchemaManager::SETTINGS_MINCOMPATIBLE_STRING,
                      TrackCollection::kRequiredSchemaVersion + 1);

    result = SchemaManager::upgradeToSchemaVersion(
            ":schema.xml", m_db, TrackCollection::kRequiredSchemaVersion);
    EXPECT_EQ(SchemaManager::RESULT_BACKWARDS_INCOMPATIBLE, result);
}

TEST_F(SchemaManagerTest, FailedUpgrade) {
    // Upgrade to version 3 to get the modern library table.
    SchemaManager::Result result = SchemaManager::upgradeToSchemaVersion(
            ":schema.xml", m_db, 3);
    EXPECT_EQ(SchemaManager::RESULT_OK, result);

    // Add a column that is added in verison 24.
    QSqlQuery query(m_db);
    EXPECT_TRUE(query.exec(
            "ALTER TABLE library ADD COLUMN coverart_source TEXT"));

    result = SchemaManager::upgradeToSchemaVersion(
            ":schema.xml", m_db, TrackCollection::kRequiredSchemaVersion);
    EXPECT_EQ(SchemaManager::RESULT_UPGRADE_FAILED, result);
}
