/***************************************************************************
                          midiobject.cpp  -  description
                             -------------------
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "midiobject.h"
#include "configobject.h"
#include "controlobject.h"
#include "controlpushbutton.h"
#include "controleventmidi.h"
#include <qevent.h>
#include <algorithm>
#include <qdir.h>

// Static member variable definition
ConfigObject<ConfigValueMidi> *MidiObject::config = 0;

/* -------- ------------------------------------------------------
   Purpose: Initialize midi, and start parsing loop
   Input:   None. Automatically selects default midi input sound
            card and device.
   Output:  -
   -------- ------------------------------------------------------ */
MidiObject::MidiObject(ConfigObject<ConfigValueMidi> *c, QApplication *a, QWidget *m, QString)
{
    app = a;
    mixxx = m;

    config = c;
    no = 0;
    requestStop = false;
}

/* -------- ------------------------------------------------------
  Purpose: Deallocates midi buffer, and closes device
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
MidiObject::~MidiObject()
{
};

void MidiObject::reopen(QString device)
{
    devClose();
    devOpen(device);
}

/* -------- ------------------------------------------------------
   Purpose: Add a control ready to recieve midi events.
   Input:   a pointer to the control. The second argument
            is the method in the control to call when the button
	    has been moved.
   Output:  -
   -------- ------------------------------------------------------ */
void MidiObject::add(ControlObject* c)
{
    controlList.push_back(c);
    no++;
//    qDebug("Registered midi control %s (%p).", c->print()->ascii(),c);
}

void MidiObject::remove(ControlObject* c)
{
    std::vector<ControlObject*>::iterator it =
        std::find(controlList.begin(), controlList.end(), c);
    if (it != controlList.end())
    {
        controlList.erase(it);
        no--;
    }
    else
        qWarning("MidiObject: Control which is requested for removal does not exist.");
}

QStringList *MidiObject::getDeviceList()
{
    return &devices;
}

QStringList *MidiObject::getConfigList(QString path)
{
    // Make sure list is empty
    configs.clear();
    
    // Get list of available midi configurations
    QDir dir(path);
    dir.setFilter(QDir::Files);
    dir.setNameFilter("*.midi.cfg *.MIDI.CFG");
    const QFileInfoList *list = dir.entryInfoList();
    if (list!=0)
    {
        QFileInfoListIterator it(*list);        // create list iterator
        QFileInfo *fi;                          // pointer for traversing
        while ((fi=it.current()))
        {
            configs.append(fi->fileName());
            ++it;   // goto next list element
        }
    }
    return &configs;
}

QString *MidiObject::getOpenDevice()
{
    return &openDevice;
}

/* -------- ------------------------------------------------------
   Purpose: Loop for parsing midi events
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
void MidiObject::send(char channel, char midicontrol, char midivalue)
{
    // Send User event, to force screen update
    //QThread::postEvent(mixxx,new ControlEventMidi(channel,midicontrol,midivalue));
};

void MidiObject::stop()
{
    requestStop = true;
}

void abortRead(int)
{
    // Reinstall default handler
    signal(SIGINT,SIG_DFL);

    // End thread execution
    QThread::exit();
}

