/**
 * @file midicontrollerpreset.h
 * @author Sean Pappalardo spappalardo@mixxx.org
 * @date Mon 9 Apr 2012
 * @brief MIDI Controller preset
 *
 * This class represents a MIDI controller preset, containing the data elements
 *   that make it up.
 *
 */

#ifndef MIDICONTROLLERPRESET_H
#define MIDICONTROLLERPRESET_H

#include <QHash>

#include "controllers/controllerpreset.h"
#include "controllers/controllerpresetvisitor.h"
#include "controllers/mixxxcontrol.h"
#include "controllers/midi/midimessage.h"

class MidiControllerPreset : public ControllerPreset {
  public:
    MidiControllerPreset() {}
    virtual ~MidiControllerPreset() {}

    virtual void accept(ControllerPresetVisitor* visitor) const {
        if (visitor) {
            visitor->visit(this);
        }
    }

    virtual bool isMappable() const {
        return true;
    }

    // Additional data elements specific to this controller type
    QHash<uint16_t, QPair<MixxxControl, MidiOptions> > mappings;
    QHash<MixxxControl, MidiOutput> outputMappings;
};

#endif
