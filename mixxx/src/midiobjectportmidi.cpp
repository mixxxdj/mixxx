/***************************************************************************
                          midiobjectportmidi.cpp  -  description
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

#include "midiobjectportmidi.h"

MidiObjectPortMidi::MidiObjectPortMidi(ConfigObject<ConfigValueMidi> *c, QApplication *a, QString device) : MidiObject(c,a,device)
{
    // Initialize PortMidi
    Pm_Initialize();

    // Fill out list holding valid input device names
    bool validDevice = false;
    for (int i=0; i<Pm_CountDevices(); i++)
    {
        const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
        if (info->input>0)
        {
            devices.append(QString(info->name));
            if (QString(info->name) == device)
                validDevice = true;
        }
    }

    if (validDevice)
        devOpen(device);
    else
        if (devices.count()==0)
            qWarning("PortMidi: No MIDI devices available.");
        else
            devOpen(devices.first());
    
    // Start the midi thread:
    start();
}

MidiObjectPortMidi::~MidiObjectPortMidi()
{
    devClose();

    //Pm_Terminate();
}

void MidiObjectPortMidi::devOpen(QString device)
{
   // Open midi device for input

    /*for (i = 0; i < Pm_CountDevices(); i++) {
        const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
        printf("%d: %s, %s", i, info->interf, info->name);
        if (info->input) printf(" (input)");
        if (info->output) printf(" (output)");
        printf("\n");
    }*/

    // Find input device with device name
    PmDeviceID id = -1;
    for (int i=0; i<Pm_CountDevices(); i++)
    {
        if (QString(Pm_GetDeviceInfo(i)->name) == device)
        {
            id = i;
            break;
        }
    }
    
    // Open device
    PmError err = Pm_OpenInput(&midi, id, NULL, 100, NULL, NULL, NULL);
    if (err)
        qDebug("PortMidi: Could not open midi device %s\n", Pm_GetErrorText(err));
    else
    {
        openDevice = device;
        start();
    }
}

void MidiObjectPortMidi::devClose()
{
    // Close device
    Pm_Close(midi);
}

void MidiObjectPortMidi::run()
{
    int stop = 0;
    PmError err;
    char midicontrol = 0;
    char midivalue = 0;

    while(stop == 0)
    {
        err = Pm_Poll(midi);
        if (err == TRUE)
        {
            if (Pm_Read(midi, buffer, 1) > 0)
            {
                midicontrol = Pm_MessageData1(buffer[0].message);
                midivalue = Pm_MessageData2(buffer[0].message);
            } else {
                qDebug("Error in Pm_Read: %s\n", Pm_GetErrorText(err));
                break;
            }
        }
        else if (err != FALSE)
        {
            qDebug("Error in Pm_Poll: %s\n", Pm_GetErrorText(err));
            break;
        }
        send(0,midicontrol,midivalue);
    }
}
