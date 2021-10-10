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

const ConfigKey mixxx::library::prefs::kKeyNotationConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("key_notation")};

const ConfigKey mixxx::library::prefs::kEditMetadataSelectedClickConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("EditMetadataSelectedClick")};

const ConfigKey mixxx::library::prefs::kSearchDebouncingTimeoutMillisConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("SearchDebouncingTimeoutMillis")};

const ConfigKey mixxx::library::prefs::kSyncTrackMetadataExportConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("SyncTrackMetadataExport")};

const ConfigKey mixxx::library::prefs::kSeratoMetadataExportConfigKey =
        ConfigKey{
                mixxx::library::prefs::kConfigGroup,
                QStringLiteral("SeratoMetadataExport")};
