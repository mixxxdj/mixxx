/***************************************************************************
                          defs_controllers.h
                          ------------------
    copyright            : (C) 2011 by Sean Pappalardo
    email                : spappalardo@mixxx.org
 ***************************************************************************/

#define USER_PRESETS_PATH QDir::homePath().append("/").append(SETTINGS_PATH).append("controllers/")
#define OLD_USER_PRESETS_PATH QDir::homePath().append("/").append(SETTINGS_PATH).append("midi/")
#define LOCAL_PRESETS_PATH QDir::homePath().append("/").append(SETTINGS_PATH).append("presets/")
#define CONTROLLER_PRESET_EXTENSION ".cntrlr.xml"
#define MIDI_MAPPING_EXTENSION ".midi.xml"
#define REQUIRED_SCRIPT_FILE "common-controller-scripts.js"
#define XML_SCHEMA_VERSION "1"
