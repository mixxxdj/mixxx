#pragma once

#include "effects/backends/effectsbackendmanager.h"
#include "preferences/usersettings.h"

/// EffectPresetManager loads and saves default EffectPresets for each type of
/// effect in the "effects/defaults" folder of the user settings folder.
class EffectPresetManager {
  public:
    EffectPresetManager(UserSettingsPointer pConfig, EffectsBackendManagerPointer pBackendManager);
    ~EffectPresetManager();

    EffectPresetPointer getDefaultPreset(EffectManifestPointer pManifest) {
        return m_defaultPresets.value(pManifest);
    }

    void saveDefaultForEffect(EffectPresetPointer pEffectPreset);
    void saveDefaultForEffect(EffectSlotPointer pEffectSlot);

  private:
    void loadDefaultEffectPresets();

    QHash<EffectManifestPointer, EffectPresetPointer> m_defaultPresets;

    UserSettingsPointer m_pConfig;
    EffectsBackendManagerPointer m_pBackendManager;
};
