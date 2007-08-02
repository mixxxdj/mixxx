/***************************************************************************
                          midiobject.h  -  description
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

#ifndef MIDIOBJECT_H
#define MIDIOBJECT_H

#include <stdio.h>
#include <stdlib.h>

#include <q3ptrvector.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <qthread.h>
#include <qobject.h>
#include <qstringlist.h>
#include <qstring.h>
#include "defs.h"
#include "configobject.h"
class ControlObject;
class QWidget;

// These enums are used in the decoding of the status message into voice categories
typedef enum {
    NOTE_OFF         = 0x80,
    NOTE_ON          = 0x90,
    AFTERTOUCH       = 0xA0,
    CTRL_CHANGE      = 0xB0,
    PROG_CHANGE      = 0xC0,
    CHANNEL_PRESSURE = 0xD0,
    PITCH_WHEEL      = 0xE0
} MidiCategory;


class MidiObject : public QThread
{
public:
    MidiObject(QString device);
    ~MidiObject();
    void setMidiConfig(ConfigObject<ConfigValueMidi> *pMidiConfig);
    void reopen(QString device);
    virtual void devOpen(QString) = 0;
    virtual void devClose() = 0;
    void add(ControlObject* c);
    void remove(ControlObject* c);
    /** Returns a list of available devices */
    QStringList *getDeviceList();
    /** Returns the name of the current open device */
    QString getOpenDevice();
    /** Returns a list of available configurations. Takes as input the directory path
      * containing the configuration files */
    QStringList *getConfigList(QString path);

	// Stuff for sending messages to control leds etc
	void sendShortMsg(unsigned char status, unsigned char byte1, unsigned char byte2);
	virtual void sendShortMsg(unsigned int word);
protected:
    void run() {};
    void stop();
    void send(MidiCategory category, char channel, char control, char value);

    bool requestStop;

    int fd, count, size, no;
    Q3PtrVector<ControlObject> controlList;

    /** List of available midi devices */
    QStringList devices;
    /** Name of current open device */
    QString openDevice;
    /** List of available midi configurations. Initialized upon call to getConfigList() */
    QStringList configs;
    /** Pointer to midi config object*/
    ConfigObject<ConfigValueMidi> *m_pMidiConfig;
};

void abortRead(int);

#endif
