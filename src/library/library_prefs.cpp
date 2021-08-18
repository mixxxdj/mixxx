#include "library/library_prefs.h"

namespace mixxx {

namespace library {

namespace prefs {

const ConfigKey kLegacyDirectoryConfigKey = ConfigKey(
        QStringLiteral("[Playlist]"),
        QStringLiteral("Directory"));

const QString kConfigGroup = QStringLiteral("[Library]");

const ConfigKey kEditMetadataSelectedClickConfigKey = ConfigKey(
        kConfigGroup,
        "EditMetadataSelectedClick");

const ConfigKey kSearchDebouncingTimeoutMillisConfigKey = ConfigKey(
        kConfigGroup,
        QStringLiteral("SearchDebouncingTimeoutMillis"));

const ConfigKey kSyncTrackMetadataExportConfigKey = ConfigKey(
        kConfigGroup,
        QStringLiteral("SyncTrackMetadataExport"));

} // namespace prefs

} // namespace library

} // namespace mixxx
