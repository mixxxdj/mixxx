#include "preferences/dialog/dlgprefautodj.h"

#include "test/mixxxtest.h"

namespace {

class AutoDJPreferencesTest : public MixxxTest {
};

TEST_F(AutoDJPreferencesTest, StartupPreferenceIsOptInAndPersistent) {
    DlgPrefAutoDJ preferences(nullptr, config());
    preferences.slotUpdate();
    EXPECT_FALSE(preferences.StartOnLaunchCheckBox->isChecked());

    preferences.StartOnLaunchCheckBox->setChecked(true);
    preferences.slotApply();
    saveAndReloadConfig();
    DlgPrefAutoDJ reloadedPreferences(nullptr, config());
    reloadedPreferences.slotUpdate();
    EXPECT_TRUE(reloadedPreferences.StartOnLaunchCheckBox->isChecked());

    reloadedPreferences.slotResetToDefaults();
    reloadedPreferences.slotApply();
    EXPECT_FALSE(config()->getValue(
            ConfigKey("[Auto DJ]", "StartOnLaunch"), true));
}

TEST_F(AutoDJPreferencesTest, CancellingRestoresStartupPreference) {
    DlgPrefAutoDJ preferences(nullptr, config());
    preferences.slotUpdate();
    preferences.StartOnLaunchCheckBox->setChecked(true);
    preferences.slotCancel();
    EXPECT_FALSE(preferences.StartOnLaunchCheckBox->isChecked());
}

} // namespace
