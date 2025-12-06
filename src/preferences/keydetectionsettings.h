#pragma once

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
#define KEY_STEM_STRATEGY "stem_strategy"
#define KEY_432HZ_DETECTION_ENABLED "432HzDetectionEnabled"
#define KEY_TUNING_DETECTION_ENABLED "TuningFrequencyDetectionEnabled"
#define KEY_TUNING_MIN_FREQUENCY "TuningMinFrequency"
#define KEY_TUNING_MAX_FREQUENCY "TuningMaxFrequency"
#define KEY_TUNING_STEP_FREQUENCY "TuningStepFrequency"

class KeyDetectionSettings {
  public:
    enum class StemStrategy {
        Disabled = 0,
        // TODO (xxx) - detect if the stem is using compliant labels for its
        // channels and use all channels but the first one if so - to be
        // implemented Automatic = 1,
        Enforced = 2
    };

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

    DEFINE_PREFERENCE_HELPERS(StemStrategy,
            StemStrategy,
            KEY_CONFIG_KEY,
            KEY_STEM_STRATEGY,
            StemStrategy::Disabled);

    DEFINE_PREFERENCE_HELPERS(Detect432Hz, bool,
                              KEY_CONFIG_KEY, KEY_432HZ_DETECTION_ENABLED, false);

    // Dynamic tuning frequency detection settings
    // When enabled, analyzes tracks across a range of reference frequencies
    // to find the most likely tuning (e.g., 432Hz, 440Hz, 442Hz)
    DEFINE_PREFERENCE_HELPERS(DetectTuningFrequency, bool,
                              KEY_CONFIG_KEY, KEY_TUNING_DETECTION_ENABLED, true);

    // Minimum tuning frequency to test (Hz) - default 427Hz (approx -52 cents from 440Hz)
    DEFINE_PREFERENCE_HELPERS(TuningMinFrequency, int,
                              KEY_CONFIG_KEY, KEY_TUNING_MIN_FREQUENCY, 427);

    // Maximum tuning frequency to test (Hz) - default 447Hz (approx +28 cents from 440Hz)
    DEFINE_PREFERENCE_HELPERS(TuningMaxFrequency, int,
                              KEY_CONFIG_KEY, KEY_TUNING_MAX_FREQUENCY, 447);

    // Step size for tuning frequency scan (Hz) - default 1Hz for accuracy
    // Higher values = faster analysis but less precision
    DEFINE_PREFERENCE_HELPERS(TuningStepFrequency, int,
                              KEY_CONFIG_KEY, KEY_TUNING_STEP_FREQUENCY, 1);

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
