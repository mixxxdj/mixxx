#ifndef PREFERENCES_KEYDETECTIONSETTINGS_H
#define PREFERENCES_KEYDETECTIONSETTINGS_H

#include "preferences/usersettings.h"

#define VAMP_CONFIG_KEY "[Vamp]"

// VAMP_CONFIG_KEY Preferences. WARNING: Do not fix the "analyser" spelling here
// since user config files contain these strings.
#define VAMP_ANALYZER_KEY_LIBRARY "AnalyserKeyLibrary"
#define VAMP_ANALYZER_KEY_PLUGIN_ID "AnalyserKeyPluginID"
#define KEY_CONFIG_KEY "[Key]"

// KEY_CONFIG_KEY Preferences
#define KEY_DETECTION_ENABLED "KeyDetectionEnabled"
#define KEY_FAST_ANALYSIS "FastAnalysisEnabled"
#define KEY_REANALYZE_WHEN_SETTINGS_CHANGE "ReanalyzeWhenSettingsChange"

#define KEY_NOTATION "KeyNotation"
#define KEY_NOTATION_OPEN_KEY "OpenKey"
#define KEY_NOTATION_OPEN_KEY_AND_TRADITIONAL "OpenKey/Traditional"
#define KEY_NOTATION_LANCELOT "Lancelot"
#define KEY_NOTATION_LANCELOT_AND_TRADITIONAL "Lancelot/Traditional"
#define KEY_NOTATION_TRADITIONAL "Traditional"
#define KEY_NOTATION_CUSTOM "Custom"
#define KEY_NOTATION_CUSTOM_PREFIX "CustomKeyNotation"

class KeyDetectionSettings {
  public:
    KeyDetectionSettings(UserSettingsPointer pConfig) : m_pConfig(pConfig) {}

    DEFINE_PREFERENCE_HELPERS(KeyDetectionEnabled, bool,
                              KEY_CONFIG_KEY, KEY_DETECTION_ENABLED, true);

    DEFINE_PREFERENCE_HELPERS(FastAnalysis, bool,
                              KEY_CONFIG_KEY, KEY_FAST_ANALYSIS, false);

    DEFINE_PREFERENCE_HELPERS(ReanalyzeWhenSettingsChange, bool,
                              KEY_CONFIG_KEY, KEY_REANALYZE_WHEN_SETTINGS_CHANGE, false);

    // TODO(rryan): Enum
    DEFINE_PREFERENCE_HELPERS(KeyNotation, QString,
                              KEY_CONFIG_KEY, KEY_NOTATION, KEY_NOTATION_TRADITIONAL);

    QString getCustomKeyNotation(mixxx::track::io::key::ChromaticKey key) {
        return m_pConfig->getValue<QString>(ConfigKey(KEY_CONFIG_KEY,
                                                      KEY_NOTATION_CUSTOM_PREFIX +
                                                      QString::number(key)));
    }

    void setCustomKeyNotation(mixxx::track::io::key::ChromaticKey key,
                              const QString& notation) {
        m_pConfig->setValue(ConfigKey(KEY_CONFIG_KEY,
                                      KEY_NOTATION_CUSTOM_PREFIX +
                                      QString::number(key)), notation);
    }

    QString getKeyPluginId() const {
        return m_pConfig->getValue<QString>(ConfigKey(
            VAMP_CONFIG_KEY, VAMP_ANALYZER_KEY_PLUGIN_ID));
    }

    void setKeyPluginId(const QString& pluginId) {
        m_pConfig->setValue(ConfigKey(
            VAMP_CONFIG_KEY, VAMP_ANALYZER_KEY_PLUGIN_ID), pluginId);
    }

  private:
    UserSettingsPointer m_pConfig;
};

#endif /* PREFERENCES_KEYDETECTIONSETTINGS_H */
