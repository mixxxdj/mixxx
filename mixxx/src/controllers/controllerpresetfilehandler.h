/**
* @file controllerpresetfilehandler.h
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Mon 9 Apr 2012
* @brief Handles loading and saving of Controller presets.
*
*/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef CONTROLLERPRESETFILEHANDLER_H
#define CONTROLLERPRESETFILEHANDLER_H

#include "xmlparse.h"
#include "controllerpreset.h"

class ControllerPresetFileHandler {

public:
    ControllerPresetFileHandler();
    ~ControllerPresetFileHandler();
    
    ControllerPreset load(const QString path, const QString deviceName,
                          const bool forceLoad);
    //virtual
    ControllerPreset load(const QDomElement root, const QString deviceName,
                                  const bool forceLoad);
    /** Given a path, saves the current preset to an XML file. */
    bool save(const ControllerPreset preset, const QString deviceName,
              const QString path);
    /** Creates the XML document and includes what script files are currently loaded.
        Sub-classes need to re-implement this (and call it first) if they
        need to add any other items. */
    virtual QDomDocument buildXML(const ControllerPreset preset, const QString deviceName);

    /** Returns just the name of a given device (everything before the first space) */
    QString rootDeviceName(QString deviceName) {
        return deviceName.left(deviceName.indexOf(" "));
    }
};
#endif