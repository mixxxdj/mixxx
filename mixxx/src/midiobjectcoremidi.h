/***************************************************************************
                          midiobjectcoremidi.h  -  description
                             -------------------
    begin                : Thu Jul 4 2002
    copyright            : (C) 2002 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MIDIOBJECTCOREMIDI_H
#define MIDIOBJECTCOREMIDI_H

#include <midiobject.h>

#include <CoreFoundation/CoreFoundation.h>
// #include <CoreAudio/CoreAudio.h>
// #include <AudioUnit/AudioUnit.h>
// #include <AudioToolbox/AUGraph.h>
// #include <CoreMIDI/CoreMIDI.h>
#include <CoreMIDI/MIDIServices.h>

/**
  *@author Tue & Ken Haste Andersen
  */

class MidiObjectCoreMidi : public MidiObject  {
public: 
    MidiObjectCoreMidi(ConfigObject<ConfigValueMidi> *c, QApplication *app, QWidget *mixxx, QString device);
    ~MidiObjectCoreMidi();
    void devOpen(QString device);
    void devClose();
    void handleMidi(const MIDIPacketList *packets);
protected:
    void run();
    void stop();

    char            *buffer;
    MIDIClientRef   midiClient;
    MIDIPortRef     midiPort;
    MIDIEndpointRef currentMidiEndpoint;
};

static void midi_read_proc(const MIDIPacketList *packets, void *refCon, void *connRefCon);

#endif
