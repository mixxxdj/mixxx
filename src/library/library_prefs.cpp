#include "library/library_prefs.h"

// Don't use nested namespaces here to ensure that both the
// definition and declaration of the constants match! Typos
// or missing definitions/declarations will then be detected
// reliably at compile time.

const ConfigKey mixxx::library::prefs::kLegacyDirectoryConfigKey =
        ConfigKey{
                QStringLiteral("[Playlist]"),
                QStringLiteral("Directory")};

const QString mixxx::library::prefs::kConfigGroup =
        QStringLiteral("[Library]");

const ConfigKey mixxx::library::prefs::kRescanOnStartupConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("RescanOnStartup")};

const ConfigKey mixxx::library::prefs::kShowScanSummaryConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("show_library_scan_summary")};

const ConfigKey mixxx::library::prefs::kKeyNotationConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("key_notation")};

const ConfigKey mixxx::library::prefs::kTrackDoubleClickActionConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("TrackLoadAction")};

const ConfigKey mixxx::library::prefs::kEditMetadataSelectedClickConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("EditMetadataSelectedClick")};

const ConfigKey mixxx::library::prefs::kHistoryMinTracksToKeepConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("history_min_tracks_to_keep")};

const ConfigKey mixxx::library::prefs::kHistoryTrackDuplicateDistanceConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("history_track_duplicate_distance")};

const ConfigKey mixxx::library::prefs::kSearchDebouncingTimeoutMillisConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("SearchDebouncingTimeoutMillis")};

const ConfigKey mixxx::library::prefs::kSearchBpmFuzzyRangeConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("search_bpm_fuzzy_range")};

const ConfigKey mixxx::library::prefs::kEnableSearchCompletionsConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("EnableSearchCompletions")};

const ConfigKey mixxx::library::prefs::kEnableSearchHistoryShortcutsConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("EnableSearchHistoryShortcuts")};

const ConfigKey mixxx::library::prefs::kBpmColumnPrecisionConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("BpmColumnPrecision")};

const ConfigKey mixxx::library::prefs::kApplyPlayedTrackColorConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("ApplyPlayedTrackColor")};

// The "Export" suffix in the key is kept for backward compatibility
const ConfigKey mixxx::library::prefs::kSyncTrackMetadataConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("SyncTrackMetadataExport")};

const ConfigKey mixxx::library::prefs::kResetMissingTagMetadataOnImportConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("ResetMissingTagMetadataOnImport")};

// The naming is unchanged for backward compatibility
const ConfigKey mixxx::library::prefs::kSyncSeratoMetadataConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("SeratoMetadataExport")};

const ConfigKey mixxx::library::prefs::kUseRelativePathOnExportConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("UseRelativePathOnExport")};

const ConfigKey mixxx::library::prefs::kCoverArtFetcherQualityConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("CoverArtFetcherQuality")};

const ConfigKey mixxx::library::prefs::kTagFetcherApplyTagsConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("TagFetcherApplyTags")};

const ConfigKey mixxx::library::prefs::kTagFetcherApplyCoverConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("TagFetcherApplyCover")};
