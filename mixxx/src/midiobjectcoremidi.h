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

// NOTE: The #include order is important: the CoreMIDI headers
//       have to be included before Qt headers (CoreMIDI uses qDebug
//       as a define, and Qt has a qDebug() function!)

#include <CoreFoundation/CoreFoundation.h>
// #include <CoreAudio/CoreAudio.h>
// #include <AudioUnit/AudioUnit.h>
// #include <AudioToolbox/AUGraph.h>
// #include <CoreMIDI/CoreMIDI.h>
#include <CoreMIDI/MIDIServices.h>

#include "midiobject.h"

#include <QtCore>

#define COREMIDI_SYSEX_QUEUE_SIZE 32 

/**
  *@author Tue & Ken Haste Andersen
  */

class MidiObjectCoreMidi : public MidiObject  {
public:
    MidiObjectCoreMidi();
    ~MidiObjectCoreMidi();
    void devOpen(QString device);
    void devClose();
    void devClose(MIDIEndpointRef ref);
    void handleMidi(const MIDIPacketList *packets, QString device);
    void makeDeviceList();
    MIDIEndpointRef getEndpoint(QString device);
    MIDIEndpointRef getDestinationEndpoint(QString device);
    void sendShortMsg(unsigned int word);
    void sendSysexMsg(unsigned char data[], unsigned int length);

    void notification_add_handler(const MIDIObjectAddRemoveNotification *message);
    void notification_remove_handler(const MIDIObjectAddRemoveNotification *message);
    void notification_property_handler(const MIDIObjectPropertyChangeNotification *message);
    void notification_handler(const MIDINotification *message);
protected:
    void run();
    void stop();

    char            *buffer;
    MIDIClientRef   midiClient;
    MIDIPortRef     midiPort;
    MIDIPortRef     midiOutPort;
    MIDIEndpointRef currentMidiEndpoint;
    MIDIEndpointRef currentMidiOutEndpoint;
    QList<MIDIEndpointRef> currentMidiEndpoints;
    QList<QString *> persistentDeviceNames;
    MIDISysexSendRequest m_sysexQueue[COREMIDI_SYSEX_QUEUE_SIZE];
    unsigned int m_sysexQueueIdx;
};

static void midi_read_proc(const MIDIPacketList *packets, void *refCon, void *connRefCon);
static CFStringRef EndpointName(MIDIEndpointRef endpoint, bool isExternal);

#endif
