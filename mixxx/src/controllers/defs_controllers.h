/***************************************************************************
                          defs_controllers.h
                          ------------------
    copyright            : (C) 2011 by Sean Pappalardo
    email                : spappalardo@mixxx.org
 ***************************************************************************/

#define USER_PRESETS_PATH QDir::homePath().append("/").append(SETTINGS_PATH).append("controllers/")
#define OLD_USER_PRESETS_PATH QDir::homePath().append("/").append(SETTINGS_PATH).append("midi/")
#define LOCAL_PRESETS_PATH QDir::homePath().append("/").append(SETTINGS_PATH).append("presets/")
#define HID_PRESET_EXTENSION ".hid.xml"
#define MIDI_PRESET_EXTENSION ".midi.xml"
#define BULK_PRESET_EXTENSION ".bulk.xml"
#define REQUIRED_SCRIPT_FILE "common-controller-scripts.js"
#define XML_SCHEMA_VERSION "1"
