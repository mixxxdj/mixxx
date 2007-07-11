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
#include <unistd.h>

MidiObjectOSS::MidiObjectOSS(QString device) : MidiObject(device)
{
    // Allocate buffer
    buffer = new unsigned char[4096];
    if (buffer == 0)
    {
        qDebug("MidiObjectOSS: Error allocating MIDI buffer");
        return;
    }

    // Fill in list of available devices
    bool device_valid = false; // Is true if device is a valid device name
    QDir dir("/dev");
    if (!dir.exists())
        qDebug( "MidiObjectOSS: Cannot find /dev directory.");
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
            if (QString("/dev/").append(fi->fileName()) == device)
                device_valid = true;
            ++it;   // goto next list element
        }
    }

    // Open device
    if (device_valid)
        devOpen(device);
    else
        if (devices.count()==0)
            qDebug("MidiObjectOSS: No MIDI devices available.");
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
    if (device.isNull())
        return;

    // Open midi device
    handle = open(device.latin1(),O_RDONLY|O_NONBLOCK);
    if (handle == -1)
    {
        qDebug("MidiObjectOSS: Open of MIDI device %s failed.",device.ascii());
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

void MidiObjectOSS::stop()
{
    MidiObject::stop();
    wait();
}

void MidiObjectOSS::run()
{
    requestStop=false;
    while(requestStop==false)
    {
        // Wait for data
        FD_ZERO(&fdset);
        FD_SET(handle, &fdset);
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 100000000;
        int r = pselect(handle+1, &fdset, NULL, NULL, &ts, 0);
        if (r>0)
        {
            int no = read(handle,&buffer[0],1);
            //qDebug("midi: %i",(short int)buffer[0]);
            if (no != 1)
                qDebug("MidiObjectOSS: midiobject recieved %i bytes. (1)", no);

            if ((buffer[0] & 128) == 128)
            {
                // and then get the following 2 bytes:
                MidiCategory midicategory;
                char midichannel;
                char midicontrol;
                char midivalue;
                int i;
                for (i=1; i<3; i++)
                {
                    int no = read(handle,&buffer[i],1);
                    if (no != 1)
                    {
                        qDebug("MidiObjectOSS: midiobject recieved %i bytes. (2)", no);
                        break;
                    }
                    if ((buffer[i] & 0xF0)>127)
                    {
                        // Somehow the sequence got mixed up, since we now receive a start
                        // of a midi command. Restart reading the command
                        buffer[0] = buffer[i];
                        i=0;
                    }
                }
                if (i==3)
                {
//           qDebug("midi oss received %i, %i, %i",buffer[0], buffer[1], buffer[2]);
                    midicategory = (MidiCategory)(buffer[0] & 0xF0);
                    midichannel = buffer[0] & 0x0F; // The channel is stored in the lower 4 bits of the status byte received
                    midicontrol = buffer[1];
                    midivalue = buffer[2];
//                     qDebug("MidiObjectOSS: midi category %i, ch: %i, ctrl: %i, val: %f",midicategory,midichannel,midicontrol,midivalue);
                    send(midicategory, midichannel, midicontrol, midivalue);
                }
            }
        }
    }
}
