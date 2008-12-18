/***************************************************************************
                          midiobjectalsaseq.h  -  description
                             -------------------
    begin                : Mon Sep 25 2006
    copyright            : (C) 2006 by Cedric GESTES
    email                : goctaf@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MIDIOBJECTPORTMIDI_H
#define MIDIOBJECTPORTMIDI_H

#include <portmidi.h>
#include <porttime.h>
#include <QtCore>
#include "midiobject.h"

#define MIXXX_PORTMIDI_BUFFER_LEN 64 /**Number of MIDI messages to buffer*/
#define MIXXX_PORTMIDI_NO_DEVICE_STRING "None" /**String to display for no MIDI devices present */
/**
  *@author Albert Santoni
  */

class MidiObjectPortMidi : public MidiObject  {
public:
    MidiObjectPortMidi();
    ~MidiObjectPortMidi();
	void updateDeviceList();
    void devOpen(QString device);
    void devClose(QString device);
    void sendShortMsg(unsigned int word);
    void sendSysexMsg(unsigned char data[], unsigned int length);
protected:
    void run();
	QString m_strActiveDevice;
	PortMidiStream *m_pInputStream;
	PortMidiStream *m_pOutputStream;
	PmEvent m_midiBuffer[MIXXX_PORTMIDI_BUFFER_LEN];
	static QList<QString> m_deviceList;
	QMutex m_mutex;
};

#endif
