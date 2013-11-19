/***************************************************************************
                          defs_controllers.h
                          ------------------
    copyright            : (C) 2011 by Sean Pappalardo
    email                : spappalardo@mixxx.org
 ***************************************************************************/

#include <QDir>
#include "configobject.h"

inline QString resourcePresetsPath(ConfigObject<ConfigValue>* pConfig) {
    QString presetsPath = pConfig->getResourcePath();
    QDir dir(presetsPath.append("/controllers/"));
    return dir.absolutePath().append("/");
}

// Prior to Mixxx 1.11.0 presets were stored in ${SETTINGS_PATH}/midi.
inline QString legacyUserPresetsPath(ConfigObject<ConfigValue>* pConfig) {
    QString presetsPath = pConfig->getSettingsPath();
    QDir dir(presetsPath.append("/midi/"));
    return dir.absolutePath().append("/");
}

inline QString userPresetsPath(ConfigObject<ConfigValue>* pConfig) {
    QString presetsPath = pConfig->getSettingsPath();
    QDir dir(presetsPath.append("/controllers/"));
    return dir.absolutePath().append("/");
}

inline QString localPresetsPath(ConfigObject<ConfigValue>* pConfig) {
    QString presetsPath = pConfig->getSettingsPath();
    QDir dir(presetsPath.append("/presets/"));
    return dir.absolutePath().append("/");
}

#define HID_PRESET_EXTENSION ".hid.xml"
#define MIDI_PRESET_EXTENSION ".midi.xml"
#define BULK_PRESET_EXTENSION ".bulk.xml"
#define REQUIRED_SCRIPT_FILE "common-controller-scripts.js"
#define XML_SCHEMA_VERSION "1"
