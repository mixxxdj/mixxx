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

MidiObjectPortMidi::MidiObjectPortMidi(ConfigObject<ConfigValueMidi> *c, QApplication *a) : MidiObject(c,a)
{
    // Open midi device for input
    Pm_Initialize();

    /*for (i = 0; i < Pm_CountDevices(); i++) {
        const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
        printf("%d: %s, %s", i, info->interf, info->name);
        if (info->input) printf(" (input)");
        if (info->output) printf(" (output)");
        printf("\n");
    }*/

    PmError err = Pm_OpenInput(&midi, 1, NULL, 100, NULL, NULL, NULL);
    if (err)
        qDebug("could not open midi device: %s\n", Pm_GetErrorText(err));

    // Start the midi thread:
    start();
}

MidiObjectPortMidi::~MidiObjectPortMidi()
{
    // Close device
    Pm_Close(midi);
}

void MidiObjectPortMidi::run()
{
    int stop = 0;
    PmError err;
    char midicontrol, midivalue;

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
        } else if (err != FALSE) {
            qDebug("Error in Pm_Poll: %s\n", Pm_GetErrorText(err));
            break;
        }
        send(0,midicontrol,midivalue);
    }
}
