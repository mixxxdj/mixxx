#pragma once

#include <QDir>
#include "preferences/usersettings.h"

inline QString resourcePresetsPath(UserSettingsPointer pConfig) {
    QString presetsPath = pConfig->getResourcePath();
    QDir dir(presetsPath.append("/controllers/"));
    return dir.absolutePath().append("/");
}

// Prior to Mixxx 1.11.0 presets were stored in ${MIXXX_SETTINGS_PATH}/midi.
inline QString legacyUserPresetsPath(UserSettingsPointer pConfig) {
    QString presetsPath = pConfig->getSettingsPath();
    QDir dir(presetsPath.append("/midi/"));
    return dir.absolutePath().append("/");
}

inline QString userPresetsPath(UserSettingsPointer pConfig) {
    QString presetsPath = pConfig->getSettingsPath();
    QDir dir(presetsPath.append("/controllers/"));
    return dir.absolutePath().append("/");
}

#define HID_PRESET_EXTENSION ".hid.xml"
#define MIDI_PRESET_EXTENSION ".midi.xml"
#define BULK_PRESET_EXTENSION ".bulk.xml"
#define REQUIRED_SCRIPT_FILE "common-controller-scripts.js"
#define XML_SCHEMA_VERSION "1"
