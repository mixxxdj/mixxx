/**
* @file midienumerator.h
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Tue 7 Feb 2012
* @brief Base class handling discovery and enumeration of DJ controllers that use the MIDI protocol.
*
* This class handles discovery and enumeration of MIDI DJ controllers and
*   must be inherited by a class that implements it on some API.
*/

#ifndef MIDIENUMERATOR_H
#define MIDIENUMERATOR_H

#include "controllers/controllerenumerator.h"
#include "controllers/midi/midicontroller.h"

class MidiEnumerator : public ControllerEnumerator {
    Q_OBJECT
  public:
    MidiEnumerator();
    virtual ~MidiEnumerator();

    virtual QList<Controller*> queryDevices() = 0;
};

#endif
