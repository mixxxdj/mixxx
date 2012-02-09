/***************************************************************************
                          defs_controllers.h
                          ------------------
    copyright            : (C) 2011 by Sean Pappalardo
    email                : spappalardo@mixxx.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define PRESETS_PATH QDir::homePath().append("/").append(SETTINGS_PATH).append("controllers/")
#define LPRESETS_PATH QDir::homePath().append("/").append(SETTINGS_PATH).append("presets/")
#define CONTROLLER_PRESET_EXTENSION ".cntrlr.xml"
#define MIDI_MAPPING_EXTENSION ".midi.xml"
#define DEFAULT_DEVICE_PRESET PRESETS_PATH.append(m_sDeviceName.replace(" ", "_") + CONTROLLER_PRESET_EXTENSION)
#define DEFAULT_MIDI_DEVICE_PRESET PRESETS_PATH.append(m_sDeviceName.right(m_sDeviceName.size()-m_sDeviceName.indexOf(" ")-1).replace(" ", "_") + MIDI_MAPPING_EXTENSION)
#define REQUIRED_SCRIPT_FILE "common-controller-scripts.js"
#define XML_SCHEMA_VERSION "1"