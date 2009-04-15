	/***************************************************************************
                          midiobjectportmidi.cpp  -  PortMidi-based MIDI backend
                           -------------------
    begin                : Wed Dec 17 2008
    copyright            : (C) 2008 Albert Santoni
    email                : alberts@mixxx.org

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
#include "midiobjectportmidi.h"

MidiObjectPortMidi::MidiObjectPortMidi() : MidiObject()
{
	m_pInputStream = NULL;
	m_pOutputStream = NULL;

    start();
}

/** Refresh the list of MIDI devices */
void MidiObjectPortMidi::updateDeviceList()
{
	int iNumDevices = Pm_CountDevices();
	devices.clear();

    const PmDeviceInfo* deviceInfo;
    for (int i = 0; i < iNumDevices; i++)
    {
        deviceInfo = Pm_GetDeviceInfo(i);

		if (deviceInfo->input)
		{
        	QString devName = QString("%1. %2").arg(QString::number(i)).arg(deviceInfo->name);
        	devices.append(devName);
		}
    }
}

MidiObjectPortMidi::~MidiObjectPortMidi()
{
	//TODO: Close the device?
        requestStop = true;
        wait();
        shutdown(); // From parent MidiObject
}

void MidiObjectPortMidi::devOpen(QString device)
{
 	const PmDeviceInfo* devInfo = NULL;
 	requestStop = false;
	int devId = -1;
	
	m_strActiveDevice = device;

	if (device != MIXXX_PORTMIDI_NO_DEVICE_STRING)
	{
		devId = device.split(".").at(0).toInt(); //Parse the # out
	}
	else
		return;

	qDebug() << "MidiObjectPortMidi: Parsed devId" << devId;

	PmError err = Pm_Initialize();
	if( err != pmNoError )
	{
		qDebug() << "PortMidi error:" << Pm_GetErrorText(err);
		return;
	}

	devInfo = Pm_GetDeviceInfo(devId);

	if (!devInfo || !devInfo->input)
	{
		devId = Pm_GetDefaultInputDeviceID();
		devInfo = Pm_GetDeviceInfo(devId);
	}


	if (devInfo)
	{
		qDebug() << "Opening" << devInfo->name;

		if (RxEnabled[device])
		{
			err = Pm_OpenInput( &m_pInputStream,
	                devId,
	                NULL, //No drive hacks
	                MIXXX_PORTMIDI_BUFFER_LEN,
	                NULL, 
	                NULL);
	
			if( err != pmNoError )
			{
				qDebug() << "PortMidi error:" << Pm_GetErrorText(err);
				return;
			}
		}
		if (TxEnabled[device])
		{
			err = Pm_OpenOutput( &m_pOutputStream,
	                devId,
	                NULL, // No driver hacks
	                0,	  // No buffering
	                NULL, // Use PortTime for timing
	                NULL, // No time info
	                0);   // No latency compensation.
	
			if( err != pmNoError )
			{
				qDebug() << "PortMidi error:" << Pm_GetErrorText(err);
				return;
			}
		}		
		
	}   
    
	//Update the list of devices...
    updateDeviceList();

    return;

}

void MidiObjectPortMidi::devClose(QString device)
{
	if (m_pInputStream)
	{
		PmError err = Pm_Close(m_pInputStream);
		if( err != pmNoError )
		{
			qDebug() << "PortMidi error:" << Pm_GetErrorText(err);
			return;
		}
	}
	
	if (m_pOutputStream)
	{
		PmError err = Pm_Close(m_pOutputStream);
		if( err != pmNoError )
		{
			qDebug() << "PortMidi error:" << Pm_GetErrorText(err);
			return;
		}
	}
	m_mutex.lock();
	requestStop = true;
	m_mutex.unlock();
}

void MidiObjectPortMidi::run()
{
    unsigned static id = 0; //the id of this thread, for debugging purposes //XXX copypasta (should factor this out somehow), -kousu 2/2009
    QThread::currentThread()->setObjectName(QString("MidiObjectPortMidi %1").arg(++id));
    
    qDebug() << QString("MidiObjectPortMidi: Thread ID=%1").arg(this->thread()->currentThreadId(),0,16);

#ifdef __MIDISCRIPT__
    MidiObject::run();
#endif

	int numEvents = 0;
	bool stopRunning = false;

	do 
	{
		if (m_pInputStream)
		{
			numEvents = Pm_Read(m_pInputStream, m_midiBuffer, MIXXX_PORTMIDI_BUFFER_LEN);
	
			for (int i = 0; i < numEvents; i++)
			{
				//if (Pm_MessageStatus(m_midiBuffer[i].message) == 0x90) //Note on, channel 1
				{
					int status = Pm_MessageStatus(m_midiBuffer[i].message);
					int type = status & 0xF0;
					int channel = status & 0x0F;
					int note = Pm_MessageData1(m_midiBuffer[i].message);
					int velocity = Pm_MessageData2(m_midiBuffer[i].message);
									
					//qDebug() << note;
	                receive((MidiCategory)type, channel, note, velocity);
				}
			}
		}
		
		usleep(5000); //Sleep for 5 milliseconds between checking for new MIDI events.
		
		m_mutex.lock();
		stopRunning = requestStop; //Cache locally for thread-safety.
		m_mutex.unlock();
		
	} while (!stopRunning);
}

void MidiObjectPortMidi::sendShortMsg(unsigned int word) {
    /*
    snd_seq_event_t ev;
    int byte1, byte2, byte3;

    // Safely retrieve the message sequence from the input
    byte1 = word & 0xff;
    byte2 = (word>>8) & 0xff;
    byte3 = (word>>16) & 0xff;
    //qDebug() << "MIDI message send via alsa seq: " << byte1 << " " << byte2 << " " << byte3;

    // Initialize the event structure
    snd_seq_ev_set_direct(&ev);
    snd_seq_ev_set_source(&ev, m_input);

    // Send to all subscribers
    snd_seq_ev_set_dest(&ev, SND_SEQ_ADDRESS_SUBSCRIBERS, 0);

    // Decide which event type to choose
    switch ((byte1 & 0xf0)) {
    case 0x90:  // Note on/off
        snd_seq_ev_set_noteon(&ev, byte1&0xf, byte2, byte3);
        snd_seq_event_output_direct(m_handle, &ev);
        break;
    case 0xb0:  // Control Change
        snd_seq_ev_set_controller(&ev, byte1&0xf, byte2, byte3);
        snd_seq_event_output_direct(m_handle, &ev);
        break;
    } */
}

// The sysex data must already contain the start byte 0xf0 and the end byte 0xf7.
void MidiObjectPortMidi::sendSysexMsg(unsigned char data[], unsigned int length) {
    /*
    snd_seq_event_t ev;

    // Initialize the event structure
    snd_seq_ev_set_direct(&ev);
    snd_seq_ev_set_source(&ev, m_input);

    // Send to all subscribers
    snd_seq_ev_set_dest(&ev, SND_SEQ_ADDRESS_SUBSCRIBERS, 0);

    // Do it
    snd_seq_ev_set_sysex(&ev,length,data);
    snd_seq_event_output_direct(m_handle, &ev);
    */
}
