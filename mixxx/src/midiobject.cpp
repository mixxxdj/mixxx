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
#include <QtDebug>
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
    // qDebug() << "Registered midi control" << c->print();
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
    dir.setNameFilter("*.midi.xml *.MIDI.XML");
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

QString MidiObject::getOpenDevice()
{
    return openDevice;
}

/* -------- ------------------------------------------------------
   Purpose: Receive a MIDI event from the backend, do value conversion as required, and queue event to mixxx control
   Input:   Values as received from MIDI
   Output:  -
   -------- ------------------------------------------------------ */
void MidiObject::send(MidiCategory category, char channel, char control, char value)
{
    // BJW: From this point onwards, use human (1-based) channel numbers
    channel++;
    // qDebug("MidiObject::send() miditype: %d ch: %d, ctrl: %d, val: %d",category, channel, control, value);
    
    MidiType type = MIDI_EMPTY;
    switch (category) {
      case NOTE_OFF:
        // BJW: Not clear why this is done.
        value = 1;
        // NB Fall-through
      case NOTE_ON:
        type = MIDI_KEY;
        break;
      case CTRL_CHANGE:
        type = MIDI_CTRL;
        break;
      case PITCH_WHEEL:
        type = MIDI_PITCH;
        break;
      default:
        type = MIDI_EMPTY;
    }

    Q_ASSERT(m_pMidiConfig);
    // qDebug("Querying action for MIDI message type=%x control=%d channel=%d", type, control, channel);
    ConfigKey *pConfigKey = m_pMidiConfig->get(ConfigValueMidi(type,control,channel));

    if (!pConfigKey) return; // No configuration was retrieved for this input event, eject.
    // qDebug() << "MidiObject::send ok" << pConfigKey->group << pConfigKey->item;
    
    ControlObject *p = ControlObject::getControl(*pConfigKey);
    ConfigOption<ConfigValueMidi> *c = m_pMidiConfig->get(*pConfigKey);

    // BJW: Apply any mapped (7-bit integer) translations
    if (c && p) {
        value = ((ConfigValueMidi *)c->val)->translateValue(value);
    }

    // BJW: newValue is the post-processed value, which may be floating point.
    double newValue = (double) value;

    // This is done separately from the switch above because it needs to go after translateValue()
    if (type == MIDI_PITCH) {
        unsigned int _14bit; 
        // Pitch bend should be 14-bit control, so control and value are multiplexed
        // (value is the MSB, control the LSB)
        // and passed as a double within the 0-127 range, but with decimal info
        // BJW: Moved here from below so that the right numbers go into ComputeValue
        _14bit = value;
        _14bit <<= 7;
        _14bit |= (unsigned char) control;
        // qDebug("-- 14 bit pitch %i", _14bit);
        // Need to force the centre point, otherwise the conversion formula maps it very slightly off-centre
        if (_14bit == 8192) 
            newValue = 63.5;
        else
            newValue = (double) _14bit * 127. / 16383.;
        // qDebug("-- converted to %f", newValue);
    }

    if (c && p)
    {
    	// qDebug("value going into ComputeValue: %f", newValue);
	newValue = ((ConfigValueMidi *)c->val)->ComputeValue(type, p->GetMidiValue(), newValue);
    	// qDebug("value coming out ComputeValue: %f", newValue);

        if (((ConfigValueMidi *)c->val)->midioption == MIDI_OPT_BUTTON || ((ConfigValueMidi *)c->val)->midioption == MIDI_OPT_SWITCH) {
           p->set(newValue);
           // qDebug("New Control Value: %g (skipping queueFromMidi call)", newValue);
           return;
        }

        // qDebug("New Control Value: %g ", newValue);
        p->queueFromMidi(category, newValue);
    }

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

void MidiObject::sendShortMsg(unsigned char status, unsigned char byte1, unsigned char byte2) {
	unsigned int word = (((unsigned int)byte2) << 16) |
		(((unsigned int)byte1) << 8) | status;
	sendShortMsg(word);
}

void MidiObject::sendShortMsg(unsigned int /* word */) {
	qDebug("MIDI message sending not implemented yet on this platform");
}
