#ifndef BEAT_PREFERENCES_H
#define BEAT_PREFERENCES_H

#define VAMP_CONFIG_KEY "[Vamp]"

// VAMP_CONFIG_KEY Preferences
#define VAMP_ANALYSER_BEAT_LIBRARY "AnalyserBeatLibrary"
#define VAMP_ANALYSER_BEAT_PLUGIN_ID "AnalyserBeatPluginID"

#define BPM_CONFIG_KEY "[BPM]"

// BPM_CONFIG_KEY Preferences
#define BPM_DETECTION_ENABLED "BPMDetectionEnabled"
#define BPM_FIXED_TEMPO_ASSUMPTION "BeatDetectionFixedTempoAssumption"
#define BPM_FIXED_TEMPO_OFFSET_CORRECTION "FixedTempoOffsetCorrection"
#define BPM_REANALYZE_WHEN_SETTINGS_CHANGE "ReanalyzeWhenSettingsChange"
#define BPM_FAST_ANALYSIS_ENABLED "FastAnalysisEnabled"

#define BPM_RANGE_START "BPMRangeStart"
#define BPM_RANGE_END "BPMRangeEnd"
#define BPM_ABOVE_RANGE_ENABLED "BPMAboveRangeEnabled"

#endif /* BEAT_PREFERENCES_H */
