/**
* @file midicontrollerpreset.h
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Mon 9 Apr 2012
* @brief MIDI Controller preset
*
* This class represents a MIDI controller preset, containing the data elements that
*   make it up.
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

#ifndef MIDICONTROLLERPRESET_H
#define MIDICONTROLLERPRESET_H

#include <QHash>

#include "configobject.h"
#include "controllers/controllerpreset.h"
#include "controllers/controllerpresetvisitor.h"
#include "controllers/midi/midimessage.h"

class MidiControllerPreset : public ControllerPreset {
  public:
    MidiControllerPreset();
    virtual ~MidiControllerPreset();

    virtual void accept(ControllerPresetVisitor* visitor) const {
        visitor->visit(this);
    }

    QHash<uint16_t, QPair<ConfigKey, MidiOptions> > mappings;
    QHash<ConfigKey, MidiOutput> outputMappings;
};

#endif
