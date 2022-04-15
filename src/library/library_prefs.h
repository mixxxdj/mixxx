#pragma once

#include "preferences/configobject.h"

namespace mixxx {

namespace library {

namespace prefs {

extern const ConfigKey kLegacyDirectoryConfigKey;

extern const QString kConfigGroup;

extern const ConfigKey kRescanOnStartupConfigKey;

extern const ConfigKey kKeyNotationConfigKey;

extern const ConfigKey kTrackDoubleClickActionConfigKey;

extern const ConfigKey kSearchDebouncingTimeoutMillisConfigKey;

extern const ConfigKey kEditMetadataSelectedClickConfigKey;

extern const ConfigKey kHistoryCleanupMinTracksConfigKey;

extern const ConfigKey kHistoryCleanupKeepLockedConfigKey;

const bool kEditMetadataSelectedClickDefault = false;

extern const ConfigKey kSyncTrackMetadataConfigKey;

extern const ConfigKey kSyncSeratoMetadataConfigKey;

extern const ConfigKey kSyncSeratoMetadataConfigKey;

extern const ConfigKey kUseRelativePathOnExportConfigKey;

} // namespace prefs

} // namespace library

} // namespace mixxx
