/**
  * @file midideviceportmidi.h
  * @author Albert Santoni alberts@mixxx.org
  * @date Thu Dec 18 2008
  * @brief PortMidi-based MIDI backend
  *
  */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MIDIDEVICEPORTMIDI_H
#define MIDIDEVICEPORTMIDI_H

#include <portmidi.h>
#include <porttime.h>
#include <QtCore>
#include "mididevice.h"

#define MIXXX_PORTMIDI_BUFFER_LEN 64 /**Number of MIDI messages to buffer*/
#define MIXXX_PORTMIDI_NO_DEVICE_STRING "None" /**String to display for no MIDI devices present */
/**
  *@author Albert Santoni
  */

/** A PortMidi-based implementation of MidiDevice */
class MidiDevicePortMidi : public MidiDevice, public QThread {
public:
    MidiDevicePortMidi(MidiMapping* mapping, 
					   const PmDeviceInfo* inputDeviceInfo, 
					   const PmDeviceInfo* outputDeviceInfo, 
					   int inputDeviceIndex,
					   int outputDeviceIndex);
    ~MidiDevicePortMidi();
    int open();
    int close();
    void sendShortMsg(unsigned int word);
    void sendSysexMsg(unsigned char data[], unsigned int length);
protected:
    void run();
    const PmDeviceInfo* m_pInputDeviceInfo;
    const PmDeviceInfo* m_pOutputDeviceInfo;
    int m_iInputDeviceIndex;
    int m_iOutputDeviceIndex;
	PortMidiStream *m_pInputStream;
	PortMidiStream *m_pOutputStream;
	PmEvent m_midiBuffer[MIXXX_PORTMIDI_BUFFER_LEN];
	static QList<QString> m_deviceList;
	QMutex m_mutex;
	bool m_bStopRequested;
};

#endif
