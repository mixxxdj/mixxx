/***************************************************************************
                          midiobjectoss.cpp  -  description
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

#include "midiobjectoss.h"
#include <qdir.h>

MidiObjectOSS::MidiObjectOSS(ConfigObject *c) : MidiObject(c)
{
    // Allocate buffer
    buffer = new char[4096];
    if (buffer == 0)
    {
        qDebug("Error allocating MIDI buffer");
        return;
    }

    // Fill in list of available devices
    QDir dir("/dev");
    if (!dir.exists())
        qWarning( "Cannot find /dev directory.");
    else
    {
        // Search for /dev/midi* devices
        dir.setFilter(QDir::System);
        dir.setNameFilter("midi*");
        const QFileInfoList *list = dir.entryInfoList();
        QFileInfoListIterator it(*list);        // create list iterator
        QFileInfo *fi;                          // pointer for traversing

        while ((fi=it.current()))
        {
            devices.append(QString("/dev/").append(fi->fileName()));
            ++it;   // goto next list element
        }
    }

    // Open default device (first in list)
    if (devices.count()==0)
        qWarning("No MIDI devices available.");
    else
        devOpen(devices.first());
}

MidiObjectOSS::~MidiObjectOSS()
{
    // Close device and delete buffer
    devClose();
    delete [] buffer;
}

void MidiObjectOSS::devOpen(QString device)
{
    // Open midi device
    handle = open(device.ascii(),0);
    if (handle == -1)
    {
        qDebug("Open of MIDI device %s failed.",device.ascii());
        return;
    }
    openDevice = device;
    start();
}

void MidiObjectOSS::devClose()
{
    stop();
    close(handle);
    openDevice = QString("");
}

void MidiObjectOSS::run()
{
    while(requestStop->available())
    {
        // First read until we get a midi channel event:
        do
        {
            int no = read(handle,&buffer[0],1);
            //qDebug("midi: %i",(short int)buffer[0]);
            if (no != 1)
                qWarning("Warning: midiobject recieved %i bytes.", no);
        } while (buffer[0] & 128 != 128); // Continue until we receive a status byte (bit 7 is set)

        // and then get the following 2 bytes:
        char channel;
        char midicontrol;
        char midivalue;
        for (int i=1; i<3; i++)
        {
            int no = read(handle,&buffer[i],1);
            if (no != 1)
                qWarning("Warning: midiobject recieved %i bytes.", no);
        }
        channel = buffer[0] & 15; // The channel is stored in the lower 4 bits of the status byte received
        midicontrol = buffer[1];
        midivalue = buffer[2];

        send(channel, midicontrol, midivalue);
        qDebug("midi");
    }
    qDebug("sem 3");
    requestStop->operator--(1);
    qDebug("sem 4");
}