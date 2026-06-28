#include "preferences/settingsmanager.h"

#include <QDir>
#include <QTemporaryDir>

#include "config.h"
#include "engine/enginebuffer.h"
#include "preferences/usersettings.h"
#include "test/mixxxtest.h"

namespace {

const ConfigKey kKeylockEngineKey(
        QStringLiteral("[App]"),
        QStringLiteral("keylock_engine"));

} // namespace

class SettingsManagerTest : public MixxxTest {};

TEST_F(SettingsManagerTest, SeedsBungeeKeylockEngineForFreshSettingsDirectory) {
    QTemporaryDir profileParent;
    ASSERT_TRUE(profileParent.isValid());

    const QString settingsPath = QDir(profileParent.path()).filePath("fresh-profile");
    ASSERT_FALSE(QDir(settingsPath).exists());

    SettingsManager manager(settingsPath);

    EXPECT_TRUE(QDir(settingsPath).exists());
    ASSERT_TRUE(manager.settings()->exists(kKeylockEngineKey));
    EXPECT_EQ(static_cast<int>(EngineBuffer::KeylockEngine::Bungee),
            manager.settings()->getValue(kKeylockEngineKey, -1));
}

TEST_F(SettingsManagerTest, DoesNotSeedBungeeKeylockEngineForExistingSettingsDirectory) {
    QTemporaryDir settingsDir;
    ASSERT_TRUE(settingsDir.isValid());
    ASSERT_TRUE(QDir(settingsDir.path()).exists());

    SettingsManager manager(settingsDir.path());

    EXPECT_FALSE(manager.settings()->exists(kKeylockEngineKey));
}

TEST_F(SettingsManagerTest, PreservesExplicitKeylockEngineInExistingSettingsDirectory) {
    QTemporaryDir settingsDir;
    ASSERT_TRUE(settingsDir.isValid());

    UserSettings existingSettings(QDir(settingsDir.path()).filePath(MIXXX_SETTINGS_FILE));
    existingSettings.setValue(
            kKeylockEngineKey,
            EngineBuffer::KeylockEngine::SoundTouch);
    ASSERT_TRUE(existingSettings.save());

    SettingsManager manager(settingsDir.path());

    ASSERT_TRUE(manager.settings()->exists(kKeylockEngineKey));
    EXPECT_EQ(static_cast<int>(EngineBuffer::KeylockEngine::SoundTouch),
            manager.settings()->getValue(kKeylockEngineKey, -1));
}
