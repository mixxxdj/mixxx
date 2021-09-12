#pragma once

#include "preferences/configobject.h"

namespace mixxx {

namespace library {

namespace prefs {

extern const ConfigKey kLegacyDirectoryConfigKey;

extern const QString kConfigGroup;

extern const ConfigKey kKeyNotationConfigKey;

extern const ConfigKey kSearchDebouncingTimeoutMillisConfigKey;

extern const ConfigKey kEditMetadataSelectedClickConfigKey;

const bool kEditMetadataSelectedClickDefault = false;

extern const ConfigKey kSyncTrackMetadataExportConfigKey;

extern const ConfigKey kSeratoMetadataExportConfigKey;

} // namespace prefs

} // namespace library

} // namespace mixxx
