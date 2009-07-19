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

// NOTE: The #include order is important, since the CoreMIDI headers
//       have to be included before Qt headers (CoreMIDI uses qDebug
//       as a define, and Qt has a qDebug() function!)
#include "midiobjectcoremidi.h"
#include <QtCore>

static QString toHex(QString numberStr) {
    return "0x" + QString("0" + QString::number(numberStr.toUShort(), 16).toUpper()).right(2);
}

/* This is a wrapper function for MidiObjectCoreMidi::notification_handler
 * needed because we cannot pass instance methods as callbacks, generally.
 * see http://www.newty.de/fpt/callback.html#chapter3 for an example of the technique
 * 
 * Assumes that you have told CoreMIDI to use 'this' as the refCon payload.
 */
void CoreMIDI_notification_handler(const MIDINotification *message, void *refCon)
{
  MidiObjectCoreMidi* self = (MidiObjectCoreMidi*)refCon; //cast out of the voidptr
  self->notification_handler(message);
}


MidiObjectCoreMidi::MidiObjectCoreMidi() : MidiObject()
{
  // Initialize CoreMidi
  MIDIClientCreate(CFSTR("Mixxx"), CoreMIDI_notification_handler, this, &midiClient);
  MIDIInputPortCreate(midiClient, CFSTR("Input port"), midi_read_proc, (void *)this, &midiPort);
  MIDIOutputPortCreate(midiClient, CFSTR("Output port"), &midiOutPort);

  // No default device opening
  //currentMidiEndpoint = MIDIGetSource(0);
  //MIDIPortConnectSource(midiPort, currentMidiEndpoint, 0);
  
  //Clear the sysex queue.
  memset(m_sysexQueue, 0, sizeof(*m_sysexQueue) * COREMIDI_SYSEX_QUEUE_SIZE);
  m_sysexQueueIdx = 0;

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
  shutdown(); // From parent MidiObject
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
  qDebug() << "Opening MIDI device " << device;
  m_deviceName = device;
  
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
  currentMidiOutEndpoint = getDestinationEndpoint(device);
  
  // Should follow selected device !!!!
  openDevices.append(device);
#ifdef __MIDISCRIPT__
	MidiObject::run();
#endif
}

void MidiObjectCoreMidi::devClose(MIDIEndpointRef ref) {
  if (!ref) return;
  
  if (currentMidiOutEndpoint != ref) {
    MIDIPortDisconnectSource(midiPort, ref);
  }
}

void MidiObjectCoreMidi::devClose()
{  
  // Find the endpoint associated with the device
  devClose(currentMidiEndpoint);

  openDevices.clear();
}

void MidiObjectCoreMidi::stop()
{
    MidiObject::stop();
}

void MidiObjectCoreMidi::run()
{
  qDebug() << "starting MidiObjectCoreMidi thread"; //XXX why is this never run????
  unsigned static id = 0; //the id of this thread, for debugging purposes //XXX copypasta (should factor this out somehow), -kousu 2/2009
  QThread::currentThread()->setObjectName(QString("MidiObjectCoreMidi %1").arg(++id));
  
  //qDebug() << QString("MidiObjectCoreMidi: Thread ID=%1").arg(this->thread()->currentThreadId(),0,16);
#ifdef __MIDISCRIPT__
  MidiObject::run();
#endif
}

void MidiObjectCoreMidi::notification_add_handler(const MIDIObjectAddRemoveNotification* message)
{
  qDebug() << "CoreMIDI_add_handler: parent:" << message->parent << ", child: " << message->child;
}

void MidiObjectCoreMidi::notification_remove_handler(const MIDIObjectAddRemoveNotification* message)
{
  qDebug() << "CoreMIDI_remove_handler: parent:" << message->parent << ", child: " << message->child;
}


void MidiObjectCoreMidi::notification_property_handler(const MIDIObjectPropertyChangeNotification* message)
{
  //unused
  //throw "NotImplemented";
  char* buf = new char[CFStringGetLength(message->propertyName)+1];
  CFStringGetCString(message->propertyName, buf, CFStringGetLength(message->propertyName)+1, kCFStringEncodingASCII);
  
  qDebug() << "CoreMIDI property changed on object " << message->object << ", propertyName=" << buf;
  
  delete buf;
  
}

/* Callback to handle informational messages from CoreMIDI
 * such as "midi device added"/"midi device removed"
 * see http://developer.apple.com/DOCUMENTATION/MusicAudio/Reference/CACoreMIDIRef/MIDIServices/
 */
void MidiObjectCoreMidi::notification_handler(const MIDINotification *message)
{
  //qDebug() << "CoreMIDI_notification_handler id:" << message->messageID << " with size " << message->messageSize;

  switch (message->messageID) {
  case kMIDIMsgSetupChanged:
    //qDebug() << "kMIDIMsgSetupChanged";
    //ignored, since we handle all the other messages
    break;
    
    /*
      case kMIDIMsgObjectAdded: //MIDIObjectAddRemoveNotification
		//qDebug() << "kMIDIMsgObjectAdded";
		notification_add_handler((MIDIObjectAddRemoveNotification*) message);
		break;
		
      case kMIDIMsgObjectRemoved: //MIDIObjectAddRemoveNotification
      ///qDebug() << "kMIDIMsgObjectRemoved";
		notification_remove_handler((MIDIObjectAddRemoveNotification*)message);
		break;
    */
    
    //hack?
  case kMIDIMsgObjectAdded:
  case kMIDIMsgObjectRemoved:
    //XXX it would be nice if we could handle add/remove separately, and we signalled "such and such" device was removed; then the prefs dialog (and anyone else listening) could be smart instead of being forced to rescan this->getDeviceList().
    makeDeviceList();
    emit(devicesChanged());
    break;
	
  case kMIDIMsgPropertyChanged: //MIDIObjectPropertyChangeNotification
    //qDebug() << "kMIDIMsgPropertyChanged";
    //notification_property_handler((MIDIObjectPropertyChangeNotification*)message);
    break;
    
    /*
    //these next two have to do with "persistent MIDI Thru connections"; not relevant to our interests, at this point
    case kMIDIMsgThruConnectionsChanged:
    qDebug() << "kMIDIMsgThruConnectionsChanged";
		break;
		
		case kMIDIMsgSerialPortOwnerChanged:
		qDebug() << "kMIDIMsgSerialPortOwnerChanged";
		break;
    */
    
  case kMIDIMsgIOError:
		//qDebug() << "kMIDIMsgIOError";
    qDebug() << "CoreMIDI driver I/O error.";
    break;
    
  default:
    qDebug() << "Unknown CoreMIDI notification: " << message->messageID;
    break;
  }
  
}


void MidiObjectCoreMidi::handleMidi(const MIDIPacketList * packets, QString device)
{
    //qDebug() << "handleMidi("<<device<<")";
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
                MidiStatusByte midistatus = (MidiStatusByte)(buffer[0] & 0xF0);
                char midichannel = buffer[0] & 0x0F; // The channel is stored in the lower 4 bits of the status byte received
                char midicontrol = buffer[1];
                char midivalue = buffer[2];

                receive(midistatus, midichannel, midicontrol, midivalue);
                messageSize = 0;
            }

            // Get data into the message array
            buffer[messageSize++] = (packet->data[j]);
        }
        packet = MIDIPacketNext(packet);
    }
    if (messageSize>0)
    {
        MidiStatusByte midistatus = (MidiStatusByte)(buffer[0] & 0xF0);
        char midichannel = buffer[0] & 0x0F;  // The channel is stored in the lower 4 bits of the status byte received
        char midicontrol = buffer[1];
        char midivalue = buffer[2];

        receive(midistatus, midichannel, midicontrol, midivalue);
    }
}

// Fill in list of available devices
void MidiObjectCoreMidi::makeDeviceList() {
    devClose();

    devices.clear();
    for (unsigned int i=0; i<MIDIGetNumberOfSources(); i++)
    {
      qDebug() << "MidiObjectCoreMidi::makeDeviceList(), at device " << i;
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

// Get a destination endpoint given a device name
MIDIEndpointRef MidiObjectCoreMidi::getDestinationEndpoint(QString device) {
        unsigned int i; // Unsigned because of comparison
        for (i = 0; i < MIDIGetNumberOfDestinations(); i++)
    {
        MIDIEndpointRef endpoint = MIDIGetDestination(i);
        CFStringRef name = EndpointName(endpoint, true);
        if (CFStringGetCStringPtr(name,0) == device)
        {
            return endpoint;
        }
    }

    qDebug() << "CoreMIDI: Error finding destination device endpoint for \"" << device
                        << "\"";
    return 0;
}

void MidiObjectCoreMidi::sendShortMsg(unsigned int word)
{
    char buf[512];
	unsigned char msg[] = { word & 0xff, (word>>8) & 0xff, (word>>16) & 0xff};

	MIDIPacketList *mpl = (MIDIPacketList*)buf;
	MIDIPacket *pkt = MIDIPacketListInit(mpl);
	pkt = MIDIPacketListAdd(mpl, sizeof(buf), pkt, 0, 3, msg);
	if (pkt)
	{
         MIDISend(midiOutPort, currentMidiOutEndpoint, mpl);
	}
	
	//qDebug() << "MidiObjectCoreMidi::sendShortMsg() " << toHex(QString::number((int)msg[0])) << toHex(QString::number((int)msg[1]))
	// 		 << toHex(QString::number((int)msg[2]));
}

/** Sends a MIDI sysex message through CoreMIDI. Copies the packet into a queue temporarily
    because we can only tell OS X to send the packets asynchronously, or in other words, we
    tell OS X we want to send a packet and then it sends it whenever it wants. Because of that,
    we need to keep the packet around until it's sent by OS X, so we throw it in a queue and
    hope it gets sent before the queue fills and wraps around to the same element. */
void MidiObjectCoreMidi::sendSysexMsg(unsigned char data[], unsigned int length) {
    
    //Grab the next element in the ringbuffer/queue.
    struct MIDISysexSendRequest* sysex = &m_sysexQueue[m_sysexQueueIdx];
  
    //Give a warning to developers if our queue overflowed. In that case, you should probably
    //up the size of the queue. (This would happen if you fire sysex messages crazy fast.)
    if (sysex->data != 0 && sysex->complete == false)
        qDebug() << "Warning: MidiObjectCoreMIDI sysex queue overflowed.";
    
    //Delete old sysex packet from our queue,if it exists.
    if (sysex->data)
        delete [] sysex->data;

    //Erase the rest of the struct.
    memset(sysex, 0, sizeof(*sysex));

    //Create our new packet, and copy over the bytes so we have them in our queue.
    sysex->destination = currentMidiOutEndpoint;
    sysex->data = new unsigned char[length];
    memcpy((void*)sysex->data, data, length * sizeof(*data));
    sysex->bytesToSend = length;

    //Send the sysex request asynchronously. We need to stick the sysex messages in a queue
    //so that when OS X actually decides to send them, the pointers (that we passed it) are 
    //still valid.
    MIDISendSysex(sysex);

    //Increment and wrap (the queue is a ringbuffer)
    m_sysexQueueIdx = (m_sysexQueueIdx+1)%COREMIDI_SYSEX_QUEUE_SIZE;
}


// C/C++ wrapper function
static void midi_read_proc(const MIDIPacketList * packets, void * refCon, void *connRefCon)
{
    //qDebug() << "midi_read_proc";
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
