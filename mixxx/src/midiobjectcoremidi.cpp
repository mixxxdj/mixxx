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

MidiObjectCoreMidi::MidiObjectCoreMidi() : MidiObject()
{
    // Initialize CoreMidi
    MIDIClientCreate(CFSTR("Mixxx"), 0, 0, &midiClient);
    MIDIInputPortCreate(midiClient, CFSTR("Input port"), midi_read_proc, (void *)this, &midiPort);

    // No default device opening
    //currentMidiEndpoint = MIDIGetSource(0);
    //MIDIPortConnectSource(midiPort, currentMidiEndpoint, 0);

    // Allocate buffer
    buffer = new char[4096];
    if (buffer == 0)
    {
        qDebug() << "Error allocating MIDI buffer";
        return;
    }

    //Make list of devices
    makeDeviceList();

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
    // Close devices and delete buffer
    for (unsigned int i=0; i<MIDIGetNumberOfSources(); i++)
    {
        devClose(MIDIGetSource(i));
    }
    delete [] buffer;
    while (!persistentDeviceNames.empty()) delete persistentDeviceNames.takeFirst();
}

void MidiObjectCoreMidi::devOpen(QString device)
{
    // Select device
	MIDIEndpointRef ref = getEndpoint(device);
	if (!ref) return;

	// Add to list of active endpoints
	currentMidiEndpoints.push_back(ref);

	// We need a pointer that will be available
	QString * persistentString = new QString(device);
	// Keep track of the pointer for deletion later
	persistentDeviceNames.append(persistentString);

    MIDIPortConnectSource(midiPort, ref, persistentString);

    // Should follow selected device !!!!
    openDevices.append(device);
}

void MidiObjectCoreMidi::devClose(MIDIEndpointRef ref) {
	if (!ref) return;

	MIDIPortDisconnectSource(midiPort, ref);
}

void MidiObjectCoreMidi::devClose(QString device)
{
	// Find the endpoint associated with the device
	MIDIEndpointRef ref = getEndpoint(device);
	devClose(ref);
	openDevices.remove(device);
}

void MidiObjectCoreMidi::stop()
{
    MidiObject::stop();
}

void MidiObjectCoreMidi::run()
{
}

void MidiObjectCoreMidi::handleMidi(const MIDIPacketList * packets, QString device)
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

                receive(midicategory, midichannel, midicontrol, midivalue, device);
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

        receive(midicategory, midichannel, midicontrol, midivalue, device);
    }
}

// Fill in list of available devices
void MidiObjectCoreMidi::makeDeviceList() {
    for (unsigned int i=0; i<MIDIGetNumberOfSources(); i++)
    {
        MIDIEndpointRef endpoint = MIDIGetSource(i);
        CFStringRef name = EndpointName(endpoint, true);
        devices.append(CFStringGetCStringPtr(name,0));
    }
}

// Get an endpoint given a device name
MIDIEndpointRef MidiObjectCoreMidi::getEndpoint(QString device) {
	unsigned int i; // Unsigned because of comparison
	for (i = 0; i < MIDIGetNumberOfSources(); i++)
    {
        MIDIEndpointRef endpoint = MIDIGetSource(i);
        CFStringRef name = EndpointName(endpoint, true);
        if (CFStringGetCStringPtr(name,0) == device)
        {
            return endpoint;
        }
    }

    qDebug() << "CoreMIDI: Error finding device endpoint for \"" << device
			<< "\"";
    return 0;
}

// C/C++ wrapper function
static void midi_read_proc(const MIDIPacketList * packets, void * refCon, void *connRefCon)
{
    MidiObjectCoreMidi * midi = (MidiObjectCoreMidi *)refCon;
    // Midi packets arrived, forward to handler with device name
    midi->handleMidi(packets, *((QString *)connRefCon));
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

/* TODO:
 * - Figure out a way of storing the endpoint, or even just the device name
 * - Retrieve the endpoint on a midi event..
 */
