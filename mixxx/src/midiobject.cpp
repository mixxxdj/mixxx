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

#include <qdir.h>
#include <qwidget.h>
#include "midiobject.h"
#include "configobject.h"
#include "controlobject.h"
#include <algorithm>


/* -------- ------------------------------------------------------
   Purpose: Initialize midi, and start parsing loop
   Input:   None. Automatically selects default midi input sound
            card and device.
   Output:  -
   -------- ------------------------------------------------------ */
MidiObject::MidiObject(QString)
{
    m_pMidiConfig = 0;
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
}

void MidiObject::setMidiConfig(ConfigObject<ConfigValueMidi> *pMidiConfig)
{
    m_pMidiConfig = pMidiConfig;
}

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
    no++;
    controlList.resize(no);
    controlList.insert(no-1,(const ControlObject *)c);
//    qDebug("Registered midi control %s (%p).", c->print()->ascii(),c);
}

void MidiObject::remove(ControlObject* c)
{
    int i = controlList.find(c,0);
    if (i>=0)
    {
        controlList.remove(i);
        no--;
    }
    else
        qDebug("MidiObject: Control which is requested for removal does not exist.");
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
#ifndef QT3_SUPPORT
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
#else
    QList<QFileInfo> list = dir.entryInfoList();
    for (int i=0; i<list.size(); ++i)
        configs.append(list.at(i).fileName());
#endif

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
void MidiObject::send(MidiCategory category, char channel, char control, double value)
{
    qDebug("MidiObject::send midi miditype: %X ch: %X, ctrl: %X, val: %g",category, channel, control, value);
    

    MidiType type = MIDI_EMPTY;
    
    switch (category) {
      case NOTE_OFF:
        qDebug("NOTE_OFF");
        value = 1.0;
      case NOTE_ON:
        type = MIDI_KEY;
        break;
      case CTRL_CHANGE:
        qDebug("CTRL_CHANGE");
        type = MIDI_CTRL;
        break;
      case PITCH_WHEEL:
        type = MIDI_PITCH;
        break;
      default:
        type = MIDI_EMPTY;
    }

    Q_ASSERT(m_pMidiConfig);
    ConfigKey *pConfigKey = m_pMidiConfig->get(ConfigValueMidi(type,control,channel));

    if (!pConfigKey) return; // No configuration was retrieved for this input event, eject.
    qDebug("MidiObject::send ok %X",pConfigKey);
    
    ControlObject *p = ControlObject::getControl(*pConfigKey);
    ConfigOption<ConfigValueMidi> *c = m_pMidiConfig->get(*pConfigKey);
    if (c && p)
    {
        value = ((ConfigValueMidi *)c->val)->ComputeValue(type, p->GetMidiValue(), value);
    }
    if (p)
        p->queueFromMidi(category, value);

}   

void MidiObject::stop()
{
    requestStop = true;
}

void abortRead(int)
{
    // Reinstall default handler
    signal(SIGINT,SIG_DFL);

#ifndef QT3_SUPPORT
    // End thread execution
    QThread::exit();
#endif
}

