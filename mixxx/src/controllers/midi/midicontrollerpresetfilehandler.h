/**
* @file midicontrollerpresetfilehandler.h
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Mon 9 Apr 2012
* @brief Handles loading and saving of MIDI controller presets.
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

#ifndef MIDICONTROLLERPRESETFILEHANDLER_H
#define MIDICONTROLLERPRESETFILEHANDLER_H

#include "controllers/controllerpresetfilehandler.h"
#include "controllers/midi/midicontrollerpreset.h"

class MidiControllerPresetFileHandler : public ControllerPresetFileHandler {
  public:
    MidiControllerPresetFileHandler();
    virtual ~MidiControllerPresetFileHandler();

    bool save(const MidiControllerPreset& preset,
              const QString deviceName, const QString fileName) const;

  private:
    virtual ControllerPreset* load(const QDomElement root, const QString deviceName,
                                   const bool forceLoad);

    void addControllerToDocument(const MidiControllerPreset& preset,
                                 const QString deviceName, QDomDocument* doc) const;

    void mappingToXML(QDomElement& parentNode,
                      QString group, QString item,
                      unsigned char status,
                      unsigned char control) const;

    void outputMappingToXML(QDomElement& parentNode, unsigned char on,
                            unsigned char off, float max, float min) const;
};

#endif
