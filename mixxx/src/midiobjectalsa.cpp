/***************************************************************************
                          midiobjectalsa.cpp  -  description
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

#include "midiobjectalsa.h"

MidiObjectALSA::MidiObjectALSA(ConfigObject *c, QApplication *a) : MidiObject(c,a)
{
    // Open midi device for input
    int card = snd_defaults_rawmidi_card();
    int device = snd_defaults_rawmidi_device();
    int err;
    if ((err = snd_rawmidi_open(&handle, card, device, SND_RAWMIDI_OPEN_INPUT)) != 0)
        qDebug("Open of midi failed: %s.", snd_strerror(err));
    else
    {
        // Allocate buffer
        buffer = new char[4096];
        if (buffer == 0)
        {
            qDebug("Midi: Error allocating buffer");
            return;
        }

        // Set number of bytes received, before snd_rawmidi_read is woken up.
        snd_rawmidi_params_t params;
        params.channel = SND_RAWMIDI_CHANNEL_INPUT;
        params.size    = 4096;
        params.min     = 1;
        err = snd_rawmidi_channel_params(handle,&params);

        // Start the midi thread:
        start();
    }
}

MidiObjectALSA::~MidiObjectALSA()
{
    // Close device
    snd_rawmidi_close(handle);

    // Deallocate buffer
    delete [] buffer;
}

void MidiObjectALSA::run()
{
    int stop = 0;
    while(stop == 0)
    {
        do
        {
            int no = snd_rawmidi_read(handle,&buffer[0],1);
            if (no != 1)
                qDebug("Warning: midiobject recieved %i bytes.", no);
        } while ((buffer[0] & 128) != 128);

        // and then get the following 2 bytes:
        char channel;
        char midicontrol;
        char midivalue;
        for (int i=1; i<3; i++)
        {
            int no = snd_rawmidi_read(handle,&buffer[i],1);
            if (no != 1)
                qDebug("Warning: midiobject recieved %i bytes.", no);
        }
        channel = buffer[0] & 15;
        midicontrol = buffer[1];
        midivalue = buffer[2];

        send(channel, midicontrol, midivalue);
    }
}
