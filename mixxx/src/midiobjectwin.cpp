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

#include "midiobjectwin.h"

MidiObjectWin::MidiObjectWin(QString device) : MidiObject(device)
{
    // Fill in list of available devices
    bool device_valid = false; // Is true if device is a valid device name
    
    MIDIINCAPS info;
    for (unsigned int i=0; i<midiInGetNumDevs(); i++)
    {
        MMRESULT res = midiInGetDevCaps(i, &info, sizeof(info));

        if (res>8)
            devices.append((const char *)info.szPname);
        else
            devices.append(QString("Device %1").arg(i));
        if (devices.last() == device)
            device_valid = true;
    }

	/*
	// Don't open the device yet, it gets opened via dlgprefmidi soon
	// This is how the ALSA one does it anyway... -Adam
    // Open device
    if (device_valid)
        devOpen(device);
    else
        if (devices.count()==0)
            qDebug("MIDI: No MIDI devices available.");
        else
            devOpen(devices.first());*/
}

MidiObjectWin::~MidiObjectWin()
{
    // Close device and delete buffer
    devClose();
}

void MidiObjectWin::devOpen(QString device)
{
    // Select device. If not found, select default (first in list).
    unsigned int i;
    MIDIINCAPS info;
    for (i=0; i<midiInGetNumDevs(); i++)
    {
        MMRESULT res = midiInGetDevCaps(i, &info, sizeof(MIDIINCAPS));
        if (res>8)
        {
            if ((const char *)info.szPname == device)
                break;
        } else {
            if (QString("Device %1").arg(i) == device)
                break;
        }
    }
    if (i==midiInGetNumDevs())
        i = 0;
    
//  handle = new HMIDIIN;
    MMRESULT res = midiInOpen(&handle, i, (DWORD)MidiInProc, (DWORD)this, CALLBACK_FUNCTION);
	if (res == MMSYSERR_NOERROR)
		// Should follow selected device !!!! 
		openDevice = device;
	
	res = midiInStart(handle);
	if (res != MMSYSERR_NOERROR)
		qDebug("Error starting midi.");
}

void MidiObjectWin::devClose()
{
    midiInReset(handle);
	midiInClose(handle);
    openDevice = QString("");
}

void MidiObjectWin::stop()
{
    MidiObject::stop();
}

void MidiObjectWin::run()
{
}

void MidiObjectWin::handleMidi(char channel, char midicontrol, char midivalue)
{
//  qDebug("midi ch: %i, ctrl: %i, val: %i",channel,midicontrol,midivalue);
    send((MidiCategory)(channel & 240), channel&15, midicontrol, midivalue);
}

// C/C++ wrapper function
void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	if (wMsg == MIM_DATA)
	{
		MidiObjectWin *midi = (MidiObjectWin*)dwInstance;
		midi->handleMidi(dwParam1&0x000000ff, (dwParam1&0x0000ff00)>>8, (dwParam1&0x00ff0000)>>16);
	}
}
    
    
    
