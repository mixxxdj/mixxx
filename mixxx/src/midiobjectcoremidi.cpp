/***************************************************************************
                          midiobjectcoremidi.cpp  -  description
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

#include <QtCore>
#include "midiobjectcoremidi.h"

MidiObjectCoreMidi::MidiObjectCoreMidi(QString device) : MidiObject(device)
{
    // Initialize CoreMidi
    MIDIClientCreate(CFSTR("Mixxx"), 0, 0, &midiClient);
    MIDIInputPortCreate(midiClient, CFSTR("Input port"), midi_read_proc, (void *)this, &midiPort);

    currentMidiEndpoint = MIDIGetSource(0);
    MIDIPortConnectSource(midiPort, currentMidiEndpoint, 0);

    // Allocate buffer
    buffer = new char[4096];
    if (buffer == 0)
    {
        qDebug() << "Error allocating MIDI buffer";
        return;
    }

    // Fill in list of available devices
    bool device_valid = false; // Is true if device is a valid device name

    for (unsigned int i=0; i<MIDIGetNumberOfSources(); i++)
    {
        MIDIEndpointRef endpoint = MIDIGetSource(i);
        CFStringRef name = EndpointName(endpoint, true);
        devices.append(CFStringGetCStringPtr(name,0));
        if (devices.last() == device)
            device_valid = true;
    }

    // Open device
/*
    if (device_valid)
        devOpen(device);
    else
        if (devices.count()==0)
            qDebug() << "CoreMidi: No MIDI devices available.";
        else
            devOpen(devices.first());
 */
}

MidiObjectCoreMidi::~MidiObjectCoreMidi()
{
    // Close device and delete buffer
    devClose();
    delete [] buffer;
}

void MidiObjectCoreMidi::devOpen(QString device)
{
    // Select device. If not found, select default (first in list).
    unsigned int i;
    for (i=0; i<MIDIGetNumberOfSources(); i++)
    {
        MIDIEndpointRef endpoint = MIDIGetSource(i);
        CFStringRef name = EndpointName(endpoint, true);
        if (CFStringGetCStringPtr(name,0) == device)
        {
            currentMidiEndpoint = MIDIGetSource(i);
            break;
        }
    }
    if (i==MIDIGetNumberOfSources())
        currentMidiEndpoint = MIDIGetSource(0);

    MIDIPortConnectSource(midiPort, currentMidiEndpoint, 0);

    // Should follow selected device !!!!
    openDevice = device;
}

void MidiObjectCoreMidi::devClose()
{
    MIDIPortDisconnectSource(midiPort, currentMidiEndpoint);
    openDevice = QString("");
}

void MidiObjectCoreMidi::stop()
{
    MidiObject::stop();
}

void MidiObjectCoreMidi::run()
{
}

void MidiObjectCoreMidi::handleMidi(const MIDIPacketList * packets)
{
    const MIDIPacket * packet;
    //Byte message[256];
    int messageSize = 0;

    // Step through each packet
    packet = packets->packet;
    for (unsigned int i=0; i<packets->numPackets; i++)
    {
        for (int j=0; j< packet->length; j++)
        {
            if (packet->data[j] >= 0xF8)
                continue; // Skip over realtime data?!?
            if ((packet->data[j] & 0x80) != 0 && messageSize > 0)
            {
                MidiCategory midicategory = (MidiCategory)(buffer[0] & 0xF0);
                char midichannel = buffer[0] & 0x0F; // The channel is stored in the lower 4 bits of the status byte received
                char midicontrol = buffer[1];
                char midivalue = buffer[2];

                send(midicategory, midichannel, midicontrol, midivalue);
                messageSize = 0;
            }

            // Get data into the message array
            buffer[messageSize++] = (packet->data[j]);
        }
        packet = MIDIPacketNext(packet);
    }
    if (messageSize>0)
    {
        MidiCategory midicategory = (MidiCategory)(buffer[0] & 0xF0);
        char midichannel = buffer[0] & 0x0F;  // The channel is stored in the lower 4 bits of the status byte received
        char midicontrol = buffer[1];
        char midivalue = buffer[2];

        send(midicategory, midichannel, midicontrol, midivalue);
    }
}

// C/C++ wrapper function
static void midi_read_proc(const MIDIPacketList * packets, void * refCon, void *)
{
    MidiObjectCoreMidi * midi = (MidiObjectCoreMidi *)refCon;
    midi->handleMidi(packets);
}

// Taken from CAMIDIEndpoints.cpp in the Apple CoreAudio SDK
// Found at /Developer/Examples/CoreAudio/PublicUtility/CAMIDIEndpoints.cpp
// Obtain the name of an endpoint without regard for whether it has connections.
// The result should be released by the caller.
static CFStringRef EndpointName(MIDIEndpointRef endpoint, bool isExternal)
{
	CFMutableStringRef result = CFStringCreateMutable(NULL, 0);
	CFStringRef str;
	
	// begin with the endpoint's name
	str = NULL;
	MIDIObjectGetStringProperty(endpoint, kMIDIPropertyName, &str);
	if (str != NULL) {
		CFStringAppend(result, str);
		CFRelease(str);
	}

	MIDIEntityRef entity = NULL;
	MIDIEndpointGetEntity(endpoint, &entity);
	if (entity == NULL)
		// probably virtual
		return result;
		
	if (CFStringGetLength(result) == 0) {
		// endpoint name has zero length -- try the entity
		str = NULL;
		MIDIObjectGetStringProperty(entity, kMIDIPropertyName, &str);
		if (str != NULL) {
			CFStringAppend(result, str);
			CFRelease(str);
		}
	}
	// now consider the device's name
	MIDIDeviceRef device = NULL;
	MIDIEntityGetDevice(entity, &device);
	if (device == NULL)
		return result;
	
	str = NULL;
	MIDIObjectGetStringProperty(device, kMIDIPropertyName, &str);
	if (str != NULL) {
		// if an external device has only one entity, throw away the endpoint name and just use the device name
		if (isExternal && MIDIDeviceGetNumberOfEntities(device) < 2) {
			CFRelease(result);
			return str;
		} else {
			// does the entity name already start with the device name? (some drivers do this though they shouldn't)
			// if so, do not prepend
			if (CFStringCompareWithOptions(str /* device name */, result /* endpoint name */, CFRangeMake(0, CFStringGetLength(str)), 0) != kCFCompareEqualTo) {
				// prepend the device name to the entity name
				if (CFStringGetLength(result) > 0)
					CFStringInsert(result, 0, CFSTR(" "));
				CFStringInsert(result, 0, str);
			}
			CFRelease(str);
		}
	}
	return result;
}

