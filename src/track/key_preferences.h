#ifndef KEY_PREFERENCES_H
#define KEY_PREFERENCES_H

#define VAMP_CONFIG_KEY "[Vamp]"

// VAMP_CONFIG_KEY Preferences. WARNING: Do not fix the "analyser" spelling here
// since user config files contain these strings.
#define VAMP_ANALYZER_KEY_LIBRARY "AnalyserKeyLibrary"
#define VAMP_ANALYZER_KEY_PLUGIN_ID "AnalyserKeyPluginID"
#define VAMP_ANALYZER_KEY_DEFAULT_PLUGIN_ID "qm-keydetector:2"

#define KEY_CONFIG_KEY "[Key]"

// KEY_CONFIG_KEY Preferences
#define KEY_DETECTION_ENABLED "KeyDetectionEnabled"
#define KEY_FAST_ANALYSIS "FastAnalysisEnabled"
#define KEY_REANALYZE_WHEN_SETTINGS_CHANGE "ReanalyzeWhenSettingsChange"

#define KEY_NOTATION "KeyNotation"
#define KEY_NOTATION_OPEN_KEY "OpenKey"
#define KEY_NOTATION_LANCELOT "Lancelot"
#define KEY_NOTATION_TRADITIONAL "Traditional"
#define KEY_NOTATION_CUSTOM "Custom"
#define KEY_NOTATION_CUSTOM_PREFIX "CustomKeyNotation"

#endif // KEY_PREFERENCES_H
