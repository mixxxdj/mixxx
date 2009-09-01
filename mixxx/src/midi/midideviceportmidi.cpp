/**
  * @file midideviceportmidi.cpp
  * @author Albert Santoni alberts@mixxx.org
  * @date Thu Dec 18 2008
  * @brief PortMidi-based MIDI backend
  * 
  * MidiDevicePortMidi is a class representing a MIDI device, either
  * physical or software. It uses the PortMidi API to send and receive
  * MIDI events to/from the device. It's important to note that PortMidi
  * treats input and output on a single physical device as two separate
  * half-duplex devices. In this class, we wrap those together into a 
  * single device, which is why the MidiDevicePortMidi constructor takes
  * both arguments pertaining to both input and output "devices". 
  *
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

#include <QtCore>
#include "configobject.h"
#include "midimapping.h"
#include "midideviceportmidi.h"


MidiDevicePortMidi::MidiDevicePortMidi(MidiMapping* mapping, 
									   const PmDeviceInfo* inputDeviceInfo, 
									   const PmDeviceInfo* outputDeviceInfo, 
									   int inputDeviceIndex,
									   int outputDeviceIndex)
									    : MidiDevice(mapping), QThread() 
{
	m_pInputStream = NULL;
	m_pOutputStream = NULL;
	m_bStopRequested = false;
	m_pInputDeviceInfo = inputDeviceInfo;
	m_pOutputDeviceInfo = outputDeviceInfo;
	m_iInputDeviceIndex = inputDeviceIndex;
	m_iOutputDeviceIndex = outputDeviceIndex;
	
	//Note: We prepend the input stream's index to the device's name to prevent duplicate devices from causing mayhem.
	m_strDeviceName = QString("%1. %2").arg(QString::number(m_iInputDeviceIndex)).arg(inputDeviceInfo->name);
	
	if (inputDeviceInfo) {
		m_bIsInputDevice = m_pInputDeviceInfo->input;
	}
	if (outputDeviceInfo) {
		m_bIsOutputDevice = m_pOutputDeviceInfo->output;
	}
}

MidiDevicePortMidi::~MidiDevicePortMidi()
{
	if (m_bIsOpen) close();
}

int MidiDevicePortMidi::open()
{
 	m_bStopRequested = false;
	
	if (m_strDeviceName == MIXXX_PORTMIDI_NO_DEVICE_STRING)
		return -1;

	PmError err = Pm_Initialize();
	if( err != pmNoError )
	{
		qDebug() << "PortMidi error:" << Pm_GetErrorText(err);
		return -1;
	}

	if (m_pInputDeviceInfo)
	{	
		if (m_bIsInputDevice)
		{
			qDebug() << "MidiObjectPortMidi: Opening" << m_pInputDeviceInfo->name << "for input";

			err = Pm_OpenInput( &m_pInputStream,
	                m_iInputDeviceIndex,
	                NULL, //No drive hacks
	                MIXXX_PORTMIDI_BUFFER_LEN,
	                NULL, 
	                NULL);
	
			if( err != pmNoError )
			{
				qDebug() << "PortMidi error:" << Pm_GetErrorText(err);
				return -2;
			}
		}
	}
	if (m_pOutputDeviceInfo)
	{
		if (m_bIsOutputDevice)
		{
			qDebug() << "MidiObjectPortMidi: Opening" << m_pOutputDeviceInfo->name << "for output";

			err = Pm_OpenOutput( &m_pOutputStream,
	                m_iOutputDeviceIndex,
	                NULL, // No driver hacks
	                0,	  // No buffering
	                NULL, // Use PortTime for timing
	                NULL, // No time info
	                0);   // No latency compensation.

			if( err != pmNoError )
			{
				qDebug() << "PortMidi error:" << Pm_GetErrorText(err);
				return -2;
			}
		}
	}   
    
    m_bIsOpen = true;
    start();

    return 0;

}

int MidiDevicePortMidi::close()
{
	m_bStopRequested = true;
	m_mutex.lock();
	
	if (m_pInputStream)
	{
		PmError err = Pm_Close(m_pInputStream);
		if( err != pmNoError )
		{
			qDebug() << "PortMidi error:" << Pm_GetErrorText(err);
			return -1;
		}
	}
	
	if (m_pOutputStream)
	{
		PmError err = Pm_Close(m_pOutputStream);
		if( err != pmNoError )
		{
			qDebug() << "PortMidi error:" << Pm_GetErrorText(err);
			return -1;
		}
	}
	
	m_bIsOpen = false;
	
	m_mutex.unlock();
		
	return 0;
}

void MidiDevicePortMidi::run()
{
	QThread::currentThread()->setObjectName(QString("MidiDevicePortMidi %1").arg(m_strDeviceName));
	int numEvents = 0;
	bool stopRunning = false;
	

	do
	{
		m_mutex.lock();
		if (m_pInputStream)
		{
			numEvents = Pm_Read(m_pInputStream, m_midiBuffer, MIXXX_PORTMIDI_BUFFER_LEN);
	
			for (int i = 0; i < numEvents; i++)
			{
				//if (Pm_MessageStatus(m_midiBuffer[i].message) == 0x90) //Note on, channel 1
				{
					unsigned char status = Pm_MessageStatus(m_midiBuffer[i].message);
					unsigned char opcode = status & 0xF0;
					unsigned char channel = status & 0x0F;
					unsigned char note = Pm_MessageData1(m_midiBuffer[i].message);
					unsigned char velocity = Pm_MessageData2(m_midiBuffer[i].message);
									
	                MidiDevice::receive((MidiStatusByte)status, channel, note, velocity);

				}
			}
		}
		
		usleep(5000); //Sleep this thread for 5 milliseconds between checking for new MIDI events.
		
		stopRunning = m_bStopRequested; //Cache locally for thread-safety.
		m_mutex.unlock(); //Have to unlock inside the loop to give the other thread a chance to lock.
		
	} while (!stopRunning);
	

}

void MidiDevicePortMidi::sendShortMsg(unsigned int word) 
{
	if (m_pOutputStream)
	{
		PmError err = Pm_WriteShort(m_pOutputStream, 0, word);
		if( err != pmNoError ) qDebug() << "PortMidi sendShortMsg error:" << Pm_GetErrorText(err);
	}
}

// The sysex data must already contain the start byte 0xf0 and the end byte 0xf7.
void MidiDevicePortMidi::sendSysexMsg(unsigned char data[], unsigned int length) 
{
	if (m_pOutputStream)
	{
		PmError err = Pm_WriteSysEx(m_pOutputStream, 0, data);
		if( err != pmNoError ) qDebug() << "PortMidi sendSysexMsg error:" << Pm_GetErrorText(err);
	} 
}
