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

MidiObjectOSS::MidiObjectOSS(ConfigObject<ConfigValueMidi> *c, QApplication *a, QString device) : MidiObject(c, a, device)
{
    thread_pid = 0;

    // Allocate buffer
    buffer = new char[4096];
    if (buffer == 0)
    {
        qDebug("Error allocating MIDI buffer");
        return;
    }

    // Fill in list of available devices
    bool device_valid = false; // Is true if device is a valid device name
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
            if (thread_pid == 0 & QString("/dev/").append(fi->fileName()) == device)
                device_valid = true;
            ++it;   // goto next list element
        }
    }

    // Open deafult device
    if (device_valid==true)
        devOpen(device);
    else
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
    handle = open(device.latin1(),0);
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
    thread_pid = getpid();
    requestStop=false;
    while(requestStop==false)
    {
        // First read until we get a midi channel event:
        do
        {
            int no = read(handle,&buffer[0],1);
            //qDebug("midi: %i",(short int)buffer[0]);
            if (no != 1)
                qWarning("Warning: midiobject recieved %i bytes.", no);
        } while ((buffer[0] & 128) != 128 & requestStop==false); // Continue until we receive a status byte (bit 7 is set)

        if (requestStop==false)
        {
            // and then get the following 2 bytes:
            char channel;
            char midicontrol;
            char midivalue;
            for (int i=1; i<3; i++)
            {
                int no = read(handle,&buffer[i],1);
                if (no != 1)
                    qWarning("Warning: midiobject recieved %i bytes.", no);
                if (requestStop==true)
                    break;
            }
            if (requestStop==false)
            {
                channel = buffer[0] & 15; // The channel is stored in the lower 4 bits of the status byte received
                midicontrol = buffer[1];
                midivalue = buffer[2];

//                qDebug("midi ch: %i, ctrl: %i, val: %i",channel,midicontrol,midivalue);
                send(channel, midicontrol, midivalue);
            }
        }
    }
}

void MidiObjectOSS::stop()
{
    MidiObject::stop();

    // Raise signal to stop abort blocking read in main thread loop
    if (thread_pid != 0)
    {
        signal(SIGINT,&abortRead);
        kill(thread_pid,SIGINT);
    }
    wait();
    thread_pid = 0;
}

void abortRead(int)
{
    // Reinstall default handler
    signal(SIGINT,SIG_DFL);

    // End thread execution
    QThread::exit();
}
