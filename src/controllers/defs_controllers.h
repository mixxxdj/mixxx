#pragma once

#include <QDir>
#include "preferences/usersettings.h"

inline QString resourceMappingsPath(UserSettingsPointer pConfig) {
    QString mappingsPath = pConfig->getResourcePath();
    QDir dir(mappingsPath.append("/controllers/"));
    return dir.absolutePath().append("/");
}

// Prior to Mixxx 1.11.0 presets were stored in ${MIXXX_SETTINGS_PATH}/midi.
inline QString legacyUserMappingsPath(UserSettingsPointer pConfig) {
    QString mappingsPath = pConfig->getSettingsPath();
    QDir dir(mappingsPath.append("/midi/"));
    return dir.absolutePath().append("/");
}

inline QString userMappingsPath(UserSettingsPointer pConfig) {
    QString mappingsPath = pConfig->getSettingsPath();
    QDir dir(mappingsPath.append("/controllers/"));
    return dir.absolutePath().append("/");
}

#define HID_MAPPING_EXTENSION ".hid.xml"
#define MIDI_MAPPING_EXTENSION ".midi.xml"
#define BULK_MAPPING_EXTENSION ".bulk.xml"
#define REQUIRED_SCRIPT_FILE "common-controller-scripts.js"
#define XML_SCHEMA_VERSION "1"
