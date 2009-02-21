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
#include <QtDebug>
#include "midiledhandler.h"
#include "midiobjectwin.h"


MidiObjectWin::MidiObjectWin() : MidiObject()
{
	updateDeviceList();

    /*
       // Don't open the device yet, it gets opened via dlgprefmidi soon
       // This is how the ALSA one does it anyway... -Adam
       // Open device
       if (device_valid)
       devOpen(device);
       else
       if (devices.count()==0)
        qDebug() << "MIDI: No MIDI devices available.";
        */
}

MidiObjectWin::~MidiObjectWin()
{
    // Close devices and delete buffer
//    while (openDevices.count() > 0) devClose(openDevices.takeFirst());
}

void MidiObjectWin::updateDeviceList() {

    // Fill in list of available devices
	devices.clear();

    MIDIINCAPS info;
    for (unsigned int i=0; i<midiInGetNumDevs(); i++)
    {
        MMRESULT res = midiInGetDevCaps(i, &info, sizeof(MIDIINCAPS));
		QString device_name= QString::fromUcs2((const ushort*)info.szPname);
        qDebug() << "Midi Device '" << device_name << "' found.";

		if (!device_name.isEmpty())
            devices.append(device_name);
        else
            devices.append(QString("Device %1").arg(i));
    }
}

void MidiObjectWin::devOpen(QString device)
{	
    if (openDevices.contains(device)) 
    	return;
    
    // Select device. If not found, select default (first in list).
    unsigned int i;
    MIDIINCAPS info;
    for (i=0; i<midiInGetNumDevs(); i++)
    {
        MMRESULT res = midiInGetDevCaps(i, &info, sizeof(MIDIINCAPS));
		QString device_name = QString::fromUcs2((const ushort*)info.szPname);
		if ((!device_name.isEmpty() && (device_name == device))|| (QString("Device %1").arg(i) == device))
        {
            qDebug() << "Using Midi Device #" << i << ": " << device_name;
            break;
        }
    }
    if (i==midiInGetNumDevs()) {
        qDebug() << "Error: Unable to find requested MIDI device " << device;
        return;
    }

    HMIDIIN handle;
    MMRESULT res = midiInOpen(&handle, i, (DWORD)MidiInProc, (DWORD) this, CALLBACK_FUNCTION);
    if (res == MMSYSERR_NOERROR) {
        // Should follow selected device !!!!
        openDevices.append(device);
    } else {
        qDebug() << "Error opening midi device";
        return;
    }

	m_deviceName = device;

    // Add device and handle to list
    handles.insert(device, handle);

    res = midiOutOpen(&outhandle, i, NULL, NULL, CALLBACK_NULL);
    if (res != MMSYSERR_NOERROR)
        qDebug() << "Error opening midi output for light control";

    res = midiInStart(handle);
    if (res != MMSYSERR_NOERROR)
        qDebug() << "Error starting midi.";
}

void MidiObjectWin::devClose()
{
    HMIDIIN handle = handles.value(m_deviceName);
    midiInReset(handle);
    midiInClose(handle);
    handles.remove(m_deviceName);
    openDevices.remove(m_deviceName);
}

void MidiObjectWin::stop()
{
    MidiObject::stop();
}

void MidiObjectWin::run()
{
    unsigned static id = 0; //the id of this thread, for debugging purposes //XXX copypasta (should factor this out somehow), -kousu 2/2009
    QThread::currentThread()->setObjectName(QString("MidiObjectWin %1").arg(++id));
    
//    qDebug() << QString("MidiObjectWin: Thread ID=%1").arg(this->thread()->currentThreadId(),0,16);
    // Set up the MidiScriptEngine here, as this is the thread the bulk of it runs in
    MidiObject::run();
}

void MidiObjectWin::handleMidi(char channel, char midicontrol, char midivalue, QString device)
{
    qDebug() << QString("midi miditype: %1 ch: %2, ctrl: %3, val: %4").arg(QString::number(channel& 240, 16).toUpper())
       .arg(QString::number(channel&15, 16).toUpper())
       .arg(QString::number(midicontrol, 16).toUpper())
       .arg(QString::number(midivalue, 16).toUpper());
    receive((MidiCategory)(channel & 240), channel&15, midicontrol, midivalue, device); // void receive(MidiCategory category, char channel, char control, char value, QString device);
}

// C/C++ wrapper function
void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
    MidiObjectWin * midi = (MidiObjectWin *)dwInstance;
    switch (wMsg) {
    case MIM_DATA:
        qDebug() << "MIM_DATA";
        midi->handleMidi(dwParam1 & 0x000000ff, (dwParam1 & 0x0000ff00) >> 8, (dwParam1 & 0x00ff0000) >> 16, midi->handles.key(hMidiIn));
	qDebug() << "MIM_DATA done.";
        break;
    case MIM_LONGDATA:
        qDebug() << "MIM_LONGDATA";
        // for a MIM_LONGDATA implementation example refer to "void CALLBACK MidiInProc" @ http://www.csee.umbc.edu/help/sound/TiMidity++-2.13.2/interface/rtsyn_winmm.c
        break;
    }
}

void MidiObjectWin::sendShortMsg(unsigned int word) {
    // This checks your compiler isn't assigning some wierd type hopefully
    DWORD raw = word;
    midiOutShortMsg(outhandle, word);
}

void MidiObjectWin::sendSysexMsg(unsigned char data[], unsigned int length)
{
    MIDIHDR header;
    memset (&header, 0, sizeof(header));
    
    header.lpData = (LPSTR)data;
    header.dwBufferLength = DWORD(length);
    header.dwBytesRecorded = DWORD(length);
    
    midiOutLongMsg(outhandle, &header, length);
}
